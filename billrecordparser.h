#pragma once
#include "bill.h"
#include <QString>

class BillRecordParser 
{
public:
    static bool parseBillLine(const QString& rawLine, Bill& bill);
    static bool parseBillDetailLine(const QString& rawLine, QString& billId, BillLine& billLine);
};
