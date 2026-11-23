#include "checkoutservice.h"
#include "billcalculator.h"
#include "order.h"
#include "orderconstants.h"
#include <QDateTime>
static const double moneyTolerance = 0.005;

CheckoutService::CheckoutService(OrderRepository& orderRepositoryToUse, BillRepository& billRepositoryToUse, ProductCatalog& productCatalogToUse, SeasonCatalog& seasonCatalogToUse)
: orderRepository(orderRepositoryToUse), billRepository(billRepositoryToUse), productCatalog(productCatalogToUse), seasonCatalog(seasonCatalogToUse)
{}

bool CheckoutService::findPreviousBill(const optional<ExistingOrderInfo>& existingOrder, Bill& previousBill)
{
    if (!existingOrder.has_value())
        return false;
    return billRepository.findBillByOrderId(existingOrder->orderId, previousBill);
}

double CheckoutService::previewSeasonalPercent(const optional<ExistingOrderInfo>& existingOrder)
{
    QDate billDate = QDate::currentDate();
    Bill previousBill;
    if (findPreviousBill(existingOrder, previousBill)) 
    {
        const QDate previousBillDate = QDate::fromString(previousBill.billDate, OrderConstants::dateFormat);
        if (previousBillDate.isValid())
            billDate = previousBillDate;
    }
    return seasonCatalog.activeDiscountPercentOn(billDate);
}

bool CheckoutService::completeCheckout(const vector<OrderItem>& orderItems, const CheckoutDetails& checkoutDetails,
const optional<ExistingOrderInfo>& existingOrder, Bill& createdBill, QString& problemDescription)
{
    if (orderItems.empty()) 
    {
        problemDescription = "The order has no products.";
        return false;
    }
    if (checkoutDetails.specialDiscountPercent < 0.0 || checkoutDetails.specialDiscountPercent > OrderConstants::maximumSpecialDiscountPercent) {
     problemDescription = "The special discount cannot be more than " + QString::number(OrderConstants::maximumSpecialDiscountPercent) + " percent.";
        return false;
    }
    const QDateTime orderMoment = QDateTime::currentDateTime();
    Bill previousBill;
    const bool orderHasPreviousBill = findPreviousBill(existingOrder, previousBill);
    QString billDateText = orderMoment.toString(OrderConstants::dateFormat);
    QString billTimeText = orderMoment.toString(OrderConstants::timeFormat);
    if (orderHasPreviousBill) 
    {
        billDateText = previousBill.billDate;
        billTimeText = previousBill.billTime;
    }
    const QDate billDate = QDate::fromString(billDateText, OrderConstants::dateFormat);
    const double seasonalPercent = seasonCatalog.activeDiscountPercentOn(billDate);
    const BillTotals totals = BillCalculator::calculateTotals(orderItems, seasonalPercent, checkoutDetails.specialDiscountPercent);
    const bool orderIsPaid = checkoutDetails.orderStatus == OrderConstants::statusPaid;
    if (orderIsPaid && checkoutDetails.amountReceived + moneyTolerance < totals.payable) {
        problemDescription = "The amount received is less than the payable amount.";
        return false;
    }
    Order order;
    if (existingOrder.has_value()) 
    {
        order.orderId = existingOrder->orderId;
        order.orderDate = existingOrder->orderDate;
        order.orderTime = existingOrder->orderTime;
    } else 
    {
        order.orderId = orderRepository.createNextOrderId(orderMoment.date());
        order.orderDate = orderMoment.toString(OrderConstants::dateFormat);
        order.orderTime = orderMoment.toString(OrderConstants::timeFormat);
    }
    order.customerName = checkoutDetails.customerName;
    order.customerContact = checkoutDetails.customerContact;
    order.status = checkoutDetails.orderStatus;
    order.paymentMode = checkoutDetails.paymentMode;
    order.paymentMerchantName = checkoutDetails.paymentMerchantName;
    order.items = orderItems;
    if (orderIsPaid)
        order.paymentAmount = totals.payable;
    bool orderWasSaved;
    if (existingOrder.has_value())
        orderWasSaved = orderRepository.replaceOrder(order, problemDescription);
    else
        orderWasSaved = orderRepository.saveOrder(order, problemDescription);
    if (!orderWasSaved)
        return false;

    map<QString, double> quantityToDeductByProductId;
    for (const OrderItem& item : orderItems)
        quantityToDeductByProductId[item.product.productId] += item.quantity;
    if (existingOrder.has_value()) 
    {
        for (const OrderItem& reservedItem : existingOrder->reservedItems)
            quantityToDeductByProductId[reservedItem.product.productId] -= reservedItem.quantity;
    }
    if (!productCatalog.adjustStock(quantityToDeductByProductId, problemDescription)) {
        problemDescription = "The order was saved but the available quantities could not be updated. " + problemDescription;
        return false;
    }

    QString billId;
    if (orderHasPreviousBill)
        billId = previousBill.billId;
    else
        billId = billRepository.createNextBillId();

    createdBill = BillCalculator::createBill(order, billId, totals, checkoutDetails.amountReceived, billDateText, billTimeText);
    bool billWasSaved;
    if (orderHasPreviousBill)
        billWasSaved = billRepository.replaceBill(createdBill, problemDescription);
    else
        billWasSaved = billRepository.saveBill(createdBill, problemDescription);
    if (!billWasSaved) {
        problemDescription = "The order was saved but the bill could not be saved. " + problemDescription;
        return false;
    }
    return true;
}
