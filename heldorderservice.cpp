#include "heldorderservice.h"
#include "billcalculator.h"
#include "filefieldformatter.h"
#include "order.h"
#include "orderconstants.h"
#include <QDateTime>

static const QString defaultCustomerName = "Walk-in";

HeldOrderService::HeldOrderService(OrderRepository& repositoryToUse, OrderReader& readerToUse,ProductCatalog& catalogToUse, BillRepository& billRepositoryToUse):
orderRepository(repositoryToUse), orderReader(readerToUse),productCatalog(catalogToUse), billRepository(billRepositoryToUse)
{}

bool HeldOrderService::holdOrder(const vector<OrderItem>& orderItems, const QString& customerName, const optional<ExistingOrderInfo>& existingOrder, QString& heldOrderId,
 QString& problemDescription)
{
    if (orderItems.empty()) 
    {
        problemDescription = "The order has no products.";
        return false;
    }
    const QDateTime holdMoment = QDateTime::currentDateTime();
    Order order;
    if (existingOrder.has_value()) 
    {
        order.orderId = existingOrder->orderId;
        order.orderDate = existingOrder->orderDate;
        order.orderTime = existingOrder->orderTime;
        order.customerContact = existingOrder->customerContact;
    } else 
    {
        order.orderId = orderRepository.createNextOrderId(holdMoment.date());
        order.orderDate = holdMoment.toString(OrderConstants::dateFormat);
        order.orderTime = holdMoment.toString(OrderConstants::timeFormat);
    }

    order.customerName = customerName.trimmed();
    if (order.customerName.isEmpty())
        order.customerName = defaultCustomerName;
    order.status = OrderConstants::statusOnHold;
    order.items = orderItems;

    bool orderWasSaved;
    if (existingOrder.has_value())
        orderWasSaved = orderRepository.replaceOrder(order, problemDescription);
    else
        orderWasSaved = orderRepository.saveOrder(order, problemDescription);
    if (!orderWasSaved)
        return false;
    heldOrderId = order.orderId;
    return true;
}

void HeldOrderService::buildItems(const vector<OrderDetailRecord>& details, vector<OrderItem>& items, QStringList& missingProducts) const
{
    for (const OrderDetailRecord& detail : details) 
    {
        const optional<Product> product = productCatalog.findByProductId(detail.productId);
        if (!product.has_value()) {
            missingProducts << detail.productBarcode;
            continue;
        }
        OrderItem item;
        item.product = *product;
        item.quantity = detail.quantity;
        items.push_back(item);
    }
}

bool HeldOrderService::buildSummaries(const vector<Order>& orders, vector<HeldOrderSummary>& summaries, QString& problemDescription)
{
    summaries.clear();
    for (size_t position = orders.size(); position > 0; --position) {
        const Order& order = orders[position - 1];

        vector<OrderDetailRecord> details;
        if (!orderReader.loadOrderDetails(order.orderId, details, problemDescription))
            return false;

        vector<OrderItem> items;
        QStringList missingProducts;
        buildItems(details, items, missingProducts);

        QStringList itemTexts;
        for (const OrderItem& item : items)
            itemTexts << item.product.acronym + " x" + FileFieldFormatter::quantity(item.quantity);

        HeldOrderSummary summary;
        summary.orderId = order.orderId;
        summary.orderDate = order.orderDate;
        summary.orderTime = order.orderTime;
        summary.customerName = order.customerName;
        summary.status = order.status;
        summary.itemSummary = itemTexts.join(", ");
        summary.totalPayable = BillCalculator::calculateTotals(items).payable;
        summary.requiresApproval = order.status == OrderConstants::statusPaid;
        summaries.push_back(summary);
    }
    return true;
}

bool HeldOrderService::findHeldOrders(vector<HeldOrderSummary>& summaries, QString& problemDescription)
{
    vector<Order> heldOrders;
    if (!orderReader.loadOrdersWithStatus(OrderConstants::statusOnHold, heldOrders, problemDescription))
        return false;
    return buildSummaries(heldOrders, summaries, problemDescription);
}

map<QString, QDate> HeldOrderService::loadPaymentDates()
{
    map<QString, QDate> paymentDateByOrderId;
    vector<Bill> bills;
    QString ignoredProblem;
    if (!billRepository.loadBills(bills, ignoredProblem))
        return paymentDateByOrderId;
    for (const Bill& bill : bills)
        paymentDateByOrderId[bill.orderId] = QDate::fromString(bill.billDate, OrderConstants::dateFormat);
    return paymentDateByOrderId;
}

QDate HeldOrderService::paymentDateOf(const Order& order, const map<QString, QDate>& paymentDateByOrderId)
{
    const auto foundDate = paymentDateByOrderId.find(order.orderId);
    if (foundDate != paymentDateByOrderId.end() && foundDate->second.isValid())
        return foundDate->second;
    return QDate::fromString(order.orderDate, OrderConstants::dateFormat);
}

bool HeldOrderService::isWithinEditWindow(const QDate& paymentDate)
{
    if (!paymentDate.isValid())
        return false;
    const qint64 daysSincePayment = paymentDate.daysTo(QDate::currentDate());
    return daysSincePayment >= 0 && daysSincePayment <= OrderConstants::paidOrderEditWindowDays;
}

bool HeldOrderService::findEditableOrders(vector<HeldOrderSummary>& summaries, QString& problemDescription)
{
    vector<Order> allOrders;
    if (!orderReader.loadAllOrders(allOrders, problemDescription))
        return false;

    const map<QString, QDate> paymentDateByOrderId = loadPaymentDates();
    vector<Order> editableOrders;
    for (const Order& order : allOrders) {
        if (order.status == OrderConstants::statusInProgress) {
            editableOrders.push_back(order);
        } else if (order.status == OrderConstants::statusPaid
                   && isWithinEditWindow(paymentDateOf(order, paymentDateByOrderId))) {
            editableOrders.push_back(order);
        }
    }
    return buildSummaries(editableOrders, summaries, problemDescription);
}

bool HeldOrderService::openOrder(const QString& orderId, HeldOrderContents& contents, QString& problemDescription)
{
    vector<Order> allOrders;
    if (!orderReader.loadAllOrders(allOrders, problemDescription))
        return false;

    const Order* foundOrder = nullptr;
    for (const Order& order : allOrders) {
        if (order.orderId == orderId)
            foundOrder = &order;
    }
    if (foundOrder == nullptr) {
        problemDescription = "Order " + orderId + " was not found.";
        return false;
    }

    const bool orderIsOnHold = foundOrder->status == OrderConstants::statusOnHold;
    const bool orderIsInProgress = foundOrder->status == OrderConstants::statusInProgress;
    const bool orderIsPaid = foundOrder->status == OrderConstants::statusPaid;
    if (!orderIsOnHold && !orderIsInProgress && !orderIsPaid) {
        problemDescription = "Order " + orderId + " is " + foundOrder->status + " and cannot be opened for editing.";
        return false;
    }
    if (orderIsPaid && !isWithinEditWindow(paymentDateOf(*foundOrder, loadPaymentDates()))) {
        problemDescription = "Order " + orderId + " was paid more than "+ QString::number(OrderConstants::paidOrderEditWindowDays) 
    + " days ago and can no longer be edited.";
        return false;
    }

    vector<OrderDetailRecord> details;
    if (!orderReader.loadOrderDetails(orderId, details, problemDescription))
        return false;

    QStringList missingProducts;
    contents.items.clear();
    contents.warning.clear();
    buildItems(details, contents.items, missingProducts);

    contents.orderInfo.orderId = foundOrder->orderId;
    contents.orderInfo.orderDate = foundOrder->orderDate;
    contents.orderInfo.orderTime = foundOrder->orderTime;
    contents.orderInfo.customerName = foundOrder->customerName;
    contents.orderInfo.customerContact = foundOrder->customerContact;
    contents.orderInfo.status = foundOrder->status;
    contents.orderInfo.paymentMode = foundOrder->paymentMode;
    contents.orderInfo.paymentMerchantName = foundOrder->paymentMerchantName;
    contents.orderInfo.reservedItems.clear();
    if (!orderIsOnHold)
        contents.orderInfo.reservedItems = contents.items;

    if (!missingProducts.isEmpty())
        contents.warning = "These products are no longer in Product.txt and were not restored: " + missingProducts.join(", ");
    return true;
}
