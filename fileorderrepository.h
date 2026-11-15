#pragma once

#include "orderrepository.h"

class FileOrderRepository : public OrderRepository {
public:
    FileOrderRepository(const QString& orderFilePathToUse, const QString& orderDetailFilePathToUse);
    QString createNextOrderId(const QDate& orderDate) override;
    bool saveOrder(const Order& order, QString& problemDescription) override;

private:
    QString orderFilePath;
    QString orderDetailFilePath;
};
