#pragma once
#include "bill.h"
#include <QString>

class BillPdfWriter 
{
public:
    static bool savePdf(const Bill& bill, const QString& filePath, QString& problemDescription);
};
