#pragma once

#include "orderitem.h"
#include "standardnames.h"

#include <QString>

struct ExistingOrderInfo {
    QString orderId;
    QString orderDate;
    QString orderTime;
    QString customerName;
    QString customerContact;
    QString status;
    QString paymentMode;
    QString paymentMerchantName;
    vector<OrderItem> reservedItems;
};
