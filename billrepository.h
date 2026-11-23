#pragma once
#include "bill.h"
#include "standardnames.h"
#include <QString>

class BillRepository 
{
public:
    virtual ~BillRepository() = default;
    virtual QString createNextBillId() = 0;
    virtual bool saveBill(const Bill& bill, QString& problemDescription) = 0;
    virtual bool replaceBill(const Bill& bill, QString& problemDescription) = 0;
    virtual bool findBillByOrderId(const QString& orderId, Bill& bill) = 0;
    virtual bool loadBills(vector<Bill>& bills, QString& problemDescription) = 0;
};
