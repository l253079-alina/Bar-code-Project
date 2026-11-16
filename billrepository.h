#pragma once
#include "bill.h"
#include <QString>

class BillRepository {
public:
    virtual ~BillRepository() = default;
    virtual QString createNextBillId() = 0;
    virtual bool saveBill(const Bill& bill, QString& problemDescription) = 0;
};
