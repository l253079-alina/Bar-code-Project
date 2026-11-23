#pragma once

#include "order.h"
#include "orderdetailrecord.h"
#include "standardnames.h"

#include <QString>

class OrderReader {
public:
    virtual ~OrderReader() = default;
    virtual bool loadOrdersWithStatus(const QString& status, vector<Order>& orders, QString& problemDescription) = 0;
    virtual bool loadAllOrders(vector<Order>& orders, QString& problemDescription) = 0;
    virtual bool loadOrderDetails(const QString& orderId, vector<OrderDetailRecord>& details, QString& problemDescription) = 0;
};
