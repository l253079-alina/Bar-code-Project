#include "reportservice.h"
#include "billcalculator.h"
#include "filefieldformatter.h"
#include "orderconstants.h"
static const size_t topItemsLimit = 10;

static bool billIsInPeriod(const Bill& bill, const QDate& fromDate, const QDate& toDate)
{
    const QDate billDate = QDate::fromString(bill.billDate, OrderConstants::dateFormat);
    if (!billDate.isValid())
        return false;
    return billDate >= fromDate && billDate <= toDate;
}

static QString periodText(const QDate& fromDate, const QDate& toDate)
{
    return fromDate.toString(OrderConstants::dateFormat) + " to " + toDate.toString(OrderConstants::dateFormat);
}

ReportService::ReportService(OrderReader& readerToUse, BillRepository& billRepositoryToUse, ProductCatalog& catalogToUse)
: orderReader(readerToUse), billRepository(billRepositoryToUse), productCatalog(catalogToUse)
{}

bool ReportService::loadBillsByOrder(map<QString, Bill>& latestBillByOrderId, QString& problemDescription)
{
    vector<Bill> bills;
    if (!billRepository.loadBills(bills, problemDescription))
        return false;
    latestBillByOrderId.clear();
    for (const Bill& bill : bills)
        latestBillByOrderId[bill.orderId] = bill;
    return true;
}

double ReportService::totalForOrder(const Order& order, const map<QString, Bill>& latestBillByOrderId, QString& problemDescription)
{
    const auto foundBill = latestBillByOrderId.find(order.orderId);
    if (foundBill != latestBillByOrderId.end())
        return foundBill->second.totalPayable;

    vector<OrderDetailRecord> details;
    if (!orderReader.loadOrderDetails(order.orderId, details, problemDescription))
        return 0.0;

    vector<OrderItem> items;
    for (const OrderDetailRecord& detail : details) {
        const optional<Product> product = productCatalog.findByProductId(detail.productId);
        if (!product.has_value())
            continue;
        OrderItem item;
        item.product = *product;
        item.quantity = detail.quantity;
        items.push_back(item);
    }
    return BillCalculator::calculateTotals(items).payable;
}

bool ReportService::incompleteOrdersReport(ReportTable& table, QString& problemDescription)
{
    vector<Order> allOrders;
    if (!orderReader.loadAllOrders(allOrders, problemDescription))
        return false;
    map<QString, Bill> latestBillByOrderId;
    if (!loadBillsByOrder(latestBillByOrderId, problemDescription))
        return false;

    table = ReportTable();
    table.headers << "Order" << "Date" << "Time" << "Customer" << "Status" << "Total (PKR)";

    int orderCount = 0;
    double grandTotal = 0.0;
    for (const Order& order : allOrders)
    {
        if (OrderConstants::isCompletedStatus(order.status))
            continue;
        const double orderTotal = totalForOrder(order, latestBillByOrderId, problemDescription);
        QStringList row;
        row << order.orderId << order.orderDate << order.orderTime << order.customerName << order.status
            << FileFieldFormatter::money(orderTotal);
        table.rows.push_back(row);
        ++orderCount;
        grandTotal += orderTotal;
    }
    table.summary = QString("Incomplete orders: %1    Total amount: PKR %2").arg(orderCount).arg(FileFieldFormatter::money(grandTotal));
    return true;
}

bool ReportService::completedOrdersReport(ReportTable& table, QString& problemDescription)
{
    vector<Order> allOrders;
    if (!orderReader.loadAllOrders(allOrders, problemDescription))
        return false;
    map<QString, Bill> latestBillByOrderId;
    if (!loadBillsByOrder(latestBillByOrderId, problemDescription))
        return false;

    table = ReportTable();
    table.headers << "Order" << "Date" << "Time" << "Customer" << "Status" << "Bill" << "Payment mode" << "Total (PKR)";

    int orderCount = 0;
    double grandTotal = 0.0;
    for (const Order& order : allOrders) {
        if (!OrderConstants::isCompletedStatus(order.status))
            continue;
        QString billId = "-";
        const auto foundBill = latestBillByOrderId.find(order.orderId);
        if (foundBill != latestBillByOrderId.end())
            billId = foundBill->second.billId;
        const double orderTotal = totalForOrder(order, latestBillByOrderId, problemDescription);

        QStringList row;
        row << order.orderId << order.orderDate << order.orderTime << order.customerName << order.status << billId
            << order.paymentMode << FileFieldFormatter::money(orderTotal);
        table.rows.push_back(row);
        ++orderCount;
        grandTotal += orderTotal;
    }
    table.summary = QString("Completed orders: %1    Total amount: PKR %2").arg(orderCount).arg(FileFieldFormatter::money(grandTotal));
    return true;
}

bool ReportService::cashSalesReport(const QDate& fromDate, const QDate& toDate, ReportTable& table, QString& problemDescription)
{
    vector<Order> allOrders;
    if (!orderReader.loadAllOrders(allOrders, problemDescription))
        return false;
    vector<Bill> bills;
    if (!billRepository.loadBills(bills, problemDescription))
        return false;

    map<QString, Order> orderById;
    for (const Order& order : allOrders)
        orderById[order.orderId] = order;
    table = ReportTable();
    table.headers << "Bill" << "Date" << "Time" << "Customer" << "Order" << "Total (PKR)";

    int saleCount = 0;
    double grandTotal = 0.0;
    for (const Bill& bill : bills) 
    {
        if (!OrderConstants::isCompletedStatus(bill.paymentStatus) || !billIsInPeriod(bill, fromDate, toDate))
            continue;
        const auto foundOrder = orderById.find(bill.orderId);
        if (foundOrder == orderById.end() || foundOrder->second.paymentMode != OrderConstants::paymentModeCash)
            continue;

        QStringList row;
        row << bill.billId << bill.billDate << bill.billTime << bill.customerName << bill.orderId << FileFieldFormatter::money(bill.totalPayable);
        table.rows.push_back(row);
        ++saleCount;
        grandTotal += bill.totalPayable;
    }
    table.summary = QString("Cash sales from %1: %2    Total amount: PKR %3").arg(periodText(fromDate, toDate)).arg(saleCount).arg(FileFieldFormatter::money(grandTotal));
    return true;
}

bool ReportService::allSalesReport(const QDate& fromDate, const QDate& toDate, ReportTable& table, QString& problemDescription)
{
    vector<Order> allOrders;
    if (!orderReader.loadAllOrders(allOrders, problemDescription))
        return false;
    vector<Bill> bills;
    if (!billRepository.loadBills(bills, problemDescription))
        return false;

    map<QString, Order> orderById;
    for (const Order& order : allOrders)
        orderById[order.orderId] = order;

    table = ReportTable();
    table.headers << "Bill" << "Date" << "Customer" << "Payment mode" << "Product discount (PKR)"<< "Seasonal discount (PKR)" << "Special discount (PKR)" << "All discounts (PKR)" << "Total (PKR)";

    int saleCount = 0;
    double grandTotal = 0.0;
    double grandDiscounts = 0.0;
    for (const Bill& bill : bills) {
        if (!OrderConstants::isCompletedStatus(bill.paymentStatus) || !billIsInPeriod(bill, fromDate, toDate))
            continue;

        QString paymentMode = "-";
        const auto foundOrder = orderById.find(bill.orderId);
        if (foundOrder != orderById.end())
            paymentMode = foundOrder->second.paymentMode;

        const double allDiscounts = bill.totalDiscount + bill.seasonalDiscount + bill.specialDiscount;
        QStringList row;
        row << bill.billId << bill.billDate << bill.customerName << paymentMode
<< FileFieldFormatter::money(bill.totalDiscount) << FileFieldFormatter::money(bill.seasonalDiscount)
<< FileFieldFormatter::money(bill.specialDiscount) << FileFieldFormatter::money(allDiscounts)
<< FileFieldFormatter::money(bill.totalPayable);
        table.rows.push_back(row);
        ++saleCount;
        grandTotal += bill.totalPayable;
        grandDiscounts += allDiscounts;
    }
    table.summary = QString("Sales from %1: %2    Total amount: PKR %3    Total discounts: PKR %4").arg(periodText(fromDate, toDate)).arg(saleCount)
.arg(FileFieldFormatter::money(grandTotal)).arg(FileFieldFormatter::money(grandDiscounts));
    return true;
}

bool ReportService::topSoldItemsReport(ReportTable& table, QString& problemDescription)
{
    vector<Bill> bills;
    if (!billRepository.loadBills(bills, problemDescription))
        return false;

    map<QString, double> soldQuantityByAcronym;
    for (const Bill& bill : bills) {
        if (!OrderConstants::isCompletedStatus(bill.paymentStatus))
            continue;
        for (const BillLine& line : bill.lines)
            soldQuantityByAcronym[line.productAcronym] += line.quantity;
    }

    vector<pair<QString, double>> soldItems(soldQuantityByAcronym.begin(), soldQuantityByAcronym.end());
    sort(soldItems.begin(), soldItems.end(),
         [](const pair<QString, double>& left, const pair<QString, double>& right) {
             if (left.second != right.second)
                 return left.second > right.second;
             return left.first < right.first;
         });

    table = ReportTable();
    table.headers << "Rank" << "Item" << "Name" << "Quantity sold" << "Remaining stock";

    const size_t shownCount = min(topItemsLimit, soldItems.size());
    for (size_t position = 0; position < shownCount; ++position) {
        QString productName = "-";
        QString remainingStock = "-";
        const optional<Product> product = productCatalog.findByAcronym(soldItems[position].first);
        if (product.has_value()) {
            productName = product->name;
            remainingStock = FileFieldFormatter::quantity(product->availableQuantity);
        }

        QStringList row;
        row << QString::number(position + 1) << soldItems[position].first << productName
            << FileFieldFormatter::quantity(soldItems[position].second) << remainingStock;
        table.rows.push_back(row);
    }
    table.summary = QString("Top %1 most sold items (completed orders)").arg(shownCount);
    return true;
}

bool ReportService::topRemainingItemsReport(ReportTable& table, QString& problemDescription)
{
    problemDescription.clear();
    vector<Product> sortedProducts = productCatalog.allProducts();
    sort(sortedProducts.begin(), sortedProducts.end(), [](const Product& left, const Product& right) {
        if (left.availableQuantity != right.availableQuantity)
            return left.availableQuantity > right.availableQuantity;
        return left.acronym < right.acronym;
    });

    table = ReportTable();
    table.headers << "Rank" << "Item" << "Name" << "Category" << "Remaining stock";

    const size_t shownCount = min(topItemsLimit, sortedProducts.size());
    for (size_t position = 0; position < shownCount; ++position) {
        const Product& product = sortedProducts[position];
        QStringList row;
        row << QString::number(position + 1) << product.acronym << product.name << product.categoryId
            << FileFieldFormatter::quantity(product.availableQuantity);
        table.rows.push_back(row);
    }
    table.summary = QString("Top %1 items with the most remaining stock, so their purchase should not be started").arg(shownCount);
    return true;
}
