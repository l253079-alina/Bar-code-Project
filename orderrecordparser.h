#pragma once

#include "order.h"
#include "orderdetailrecord.h"

#include <QString>

class OrderRecordParser {
public:
    static bool parseOrderLine(const QString& rawLine, Order& order);
    static bool parseOrderDetailLine(const QString& rawLine, OrderDetailRecord& detail);
};
