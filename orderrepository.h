#pragma once

#include "order.h"

#include <QDate>
#include <QString>

class OrderRepository {
public:
    virtual ~OrderRepository() = default;
    virtual QString createNextOrderId(const QDate& orderDate) = 0;
    virtual bool saveOrder(const Order& order, QString& problemDescription) = 0;
    virtual bool replaceOrder(const Order& order, QString& problemDescription) = 0;
};
