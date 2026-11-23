#include "orderrecordparser.h"

#include "filefieldformatter.h"

static const int orderFieldCount = 9;
static const int orderDetailFieldCount = 4;

bool OrderRecordParser::parseOrderLine(const QString& rawLine, Order& order)
{
    const QStringList fields = FileFieldFormatter::fieldsOfLine(rawLine);
    if (fields.size() != orderFieldCount)
        return false;

    bool paymentAmountIsNumber = false;
    const double paymentAmount = fields[6].toDouble(&paymentAmountIsNumber);
    if (!paymentAmountIsNumber)
        return false;

    order.orderId = fields[0];
    order.orderDate = fields[1];
    order.orderTime = fields[2];
    order.customerName = FileFieldFormatter::restoreText(fields[3]);
    order.customerContact = FileFieldFormatter::restoreText(fields[4]);
    order.status = fields[5];
    order.paymentAmount = paymentAmount;
    order.paymentMode = FileFieldFormatter::restoreText(fields[7]);
    order.paymentMerchantName = FileFieldFormatter::restoreText(fields[8]);
    return true;
}

bool OrderRecordParser::parseOrderDetailLine(const QString& rawLine, OrderDetailRecord& detail)
{
    const QStringList fields = FileFieldFormatter::fieldsOfLine(rawLine);
    if (fields.size() != orderDetailFieldCount)
        return false;

    bool quantityIsNumber = false;
    const double quantity = fields[3].toDouble(&quantityIsNumber);
    if (!quantityIsNumber)
        return false;

    detail.orderId = fields[0];
    detail.productId = fields[1];
    detail.productBarcode = fields[2];
    detail.quantity = quantity;
    return true;
}
