#pragma once
#include "billrepository.h"

class FileBillRepository : public BillRepository {
public:
    explicit FileBillRepository(const QString& billFilePathToUse): billFilePath(billFilePathToUse) {}
    QString createNextBillId() override;
    bool saveBill(const Bill& bill, QString& problemDescription) override;

private:
    QString billFilePath;
};
