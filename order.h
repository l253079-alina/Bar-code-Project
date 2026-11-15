#pragma once

#include "orderitem.h"
#include "standardnames.h"

#include <QString>

struct Order {
    QString orderId;
    QString orderDate;
    QString orderTime;
    QString customerName;
    QString customerContact;
    QString status;
    double paymentAmount = 0.0;
    QString paymentMode;
    QString paymentMerchantName;
    vector<OrderItem> items;
};
