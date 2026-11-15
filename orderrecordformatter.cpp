#include "orderrecordformatter.h"

#include "filefieldformatter.h"

QString OrderRecordFormatter::orderLine(const Order& order)
{
    QStringList fields;
    fields << order.orderId
           << order.orderDate
           << order.orderTime
           << FileFieldFormatter::text(order.customerName)
           << FileFieldFormatter::text(order.customerContact)
           << FileFieldFormatter::text(order.status)
           << FileFieldFormatter::money(order.paymentAmount)
           << FileFieldFormatter::text(order.paymentMode)
           << FileFieldFormatter::text(order.paymentMerchantName);
    return FileFieldFormatter::lineWithEndMarker(fields);
}

QStringList OrderRecordFormatter::orderDetailLines(const Order& order)
{
    QStringList detailLines;
    for (const OrderItem& item : order.items) {
        QStringList fields;
        fields << order.orderId
               << item.product.productId
               << item.product.barcode
               << FileFieldFormatter::quantity(item.quantity);
        detailLines.append(FileFieldFormatter::lineWithEndMarker(fields));
    }
    return detailLines;
}
