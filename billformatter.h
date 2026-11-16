#pragma once
#include "bill.h"
#include <QString>

class BillFormatter {
public:
    static QString fileLine(const Bill& bill);
    static QString screenText(const Bill& bill);
};
