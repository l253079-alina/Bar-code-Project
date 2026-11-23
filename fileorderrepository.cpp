#include "fileorderrepository.h"

#include "orderrecordformatter.h"
#include "orderrecordparser.h"
#include "textfilelines.h"

static const int orderIdLength = 8;
static const int orderSequenceLength = 6;
static const int orderDayLength = 2;

FileOrderRepository::FileOrderRepository(const QString& orderFilePathToUse, const QString& orderDetailFilePathToUse)
    : orderFilePath(orderFilePathToUse), orderDetailFilePath(orderDetailFilePathToUse)
{
}

QString FileOrderRepository::createNextOrderId(const QDate& orderDate)
{
    const int highestSequenceNumber =
        TextFileLines::highestIdNumber(orderFilePath, orderIdLength, orderSequenceLength);
    const int nextSequenceNumber = highestSequenceNumber + 1;
    return QString("%1%2")
        .arg(nextSequenceNumber, orderSequenceLength, 10, QChar('0'))
        .arg(orderDate.day(), orderDayLength, 10, QChar('0'));
}

bool FileOrderRepository::saveOrder(const Order& order, QString& problemDescription)
{
    const QStringList orderLines = QStringList() << OrderRecordFormatter::orderLine(order);
    if (!TextFileLines::appendLines(orderFilePath, orderLines, problemDescription))
        return false;

    return TextFileLines::appendLines(orderDetailFilePath, OrderRecordFormatter::orderDetailLines(order),
                                      problemDescription);
}

bool FileOrderRepository::replaceOrder(const Order& order, QString& problemDescription)
{
    QStringList orderLines = TextFileLines::readNonEmptyLines(orderFilePath);
    bool orderWasFound = false;
    for (QString& orderLine : orderLines) {
        if (orderLine.section(' ', 0, 0) == order.orderId) {
            orderLine = OrderRecordFormatter::orderLine(order);
            orderWasFound = true;
        }
    }
    if (!orderWasFound) {
        problemDescription = "Order " + order.orderId + " was not found in " + orderFilePath;
        return false;
    }
    if (!TextFileLines::writeAllLines(orderFilePath, orderLines, problemDescription))
        return false;

    QStringList keptDetailLines;
    for (const QString& detailLine : TextFileLines::readNonEmptyLines(orderDetailFilePath)) {
        if (detailLine.section(' ', 0, 0) != order.orderId)
            keptDetailLines.append(detailLine);
    }
    keptDetailLines.append(OrderRecordFormatter::orderDetailLines(order));
    return TextFileLines::writeAllLines(orderDetailFilePath, keptDetailLines, problemDescription);
}

bool FileOrderRepository::loadOrdersWithStatus(const QString& status, vector<Order>& orders, QString& problemDescription)
{
    orders.clear();
    problemDescription.clear();
    for (const QString& orderLine : TextFileLines::readNonEmptyLines(orderFilePath)) {
        Order order;
        if (OrderRecordParser::parseOrderLine(orderLine, order) && order.status == status)
            orders.push_back(order);
    }
    return true;
}

bool FileOrderRepository::loadAllOrders(vector<Order>& orders, QString& problemDescription)
{
    orders.clear();
    problemDescription.clear();
    for (const QString& orderLine : TextFileLines::readNonEmptyLines(orderFilePath)) {
        Order order;
        if (OrderRecordParser::parseOrderLine(orderLine, order))
            orders.push_back(order);
    }
    return true;
}

bool FileOrderRepository::loadOrderDetails(const QString& orderId, vector<OrderDetailRecord>& details, QString& problemDescription)
{
    details.clear();
    problemDescription.clear();
    for (const QString& detailLine : TextFileLines::readNonEmptyLines(orderDetailFilePath)) {
        OrderDetailRecord detail;
        if (OrderRecordParser::parseOrderDetailLine(detailLine, detail) && detail.orderId == orderId)
            details.push_back(detail);
    }
    return true;
}
