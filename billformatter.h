#pragma once
#include "bill.h"
#include <QString>
#include <QStringList>

class BillFormatter 
{
public:
    static QString fileLine(const Bill& bill);
    static QStringList detailLines(const Bill& bill);
    static QString screenText(const Bill& bill);
};
