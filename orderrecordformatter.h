#pragma once

#include "order.h"

#include <QString>
#include <QStringList>

class OrderRecordFormatter {
public:
    static QString orderLine(const Order& order);
    static QStringList orderDetailLines(const Order& order);
};
