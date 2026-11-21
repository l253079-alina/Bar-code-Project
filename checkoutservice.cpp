#include "checkoutservice.h"
#include "billcalculator.h"
#include "order.h"
#include "orderconstants.h"
#include <QDateTime>

CheckoutService::CheckoutService(OrderRepository& orderRepositoryToUse, BillRepository& billRepositoryToUse, ProductCatalog& productCatalogToUse)
: orderRepository(orderRepositoryToUse), billRepository(billRepositoryToUse), productCatalog(productCatalogToUse)
{}

bool CheckoutService::completeCheckout(const vector<OrderItem>& orderItems, const CheckoutDetails& checkoutDetails, Bill& createdBill, QString& problemDescription)
{
    problemDescription.clear();
    if (orderItems.empty()) 
    {
        problemDescription = "The order has no products.";
        return false;
    }

    const QDateTime orderMoment = QDateTime::currentDateTime();
    const BillTotals totals = BillCalculator::calculateTotals(orderItems);

    Order order;
    order.orderId = orderRepository.createNextOrderId(orderMoment.date());
    order.orderDate = orderMoment.toString("yyyy-MM-dd");
    order.orderTime = orderMoment.toString("HH:mm:ss");
    order.customerName = checkoutDetails.customerName;
    order.customerContact = checkoutDetails.customerContact;
    order.status = checkoutDetails.orderStatus;
    order.paymentMode = checkoutDetails.paymentMode;
    order.paymentMerchantName = checkoutDetails.paymentMerchantName;
    order.items = orderItems;
    if (order.status == OrderConstants::statusPaid)
        order.paymentAmount = totals.payable;

    if (!orderRepository.saveOrder(order, problemDescription))
        return false;

    createdBill = BillCalculator::createBill(order, billRepository.createNextBillId());
    if (!billRepository.saveBill(createdBill, problemDescription)) 
    {
        problemDescription = "The order was saved but the bill could not be saved. " + problemDescription;
        return false;
    }

    if (order.status == OrderConstants::statusPaid) 
    {
        QString stockProblemDescription;
        if (!productCatalog.reduceStock(orderItems, stockProblemDescription))
            problemDescription = "The order and the bill were saved but the stock in Product.txt could not be updated. " + stockProblemDescription;
    }
    return true;
}
