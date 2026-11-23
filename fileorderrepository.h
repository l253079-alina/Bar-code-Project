#pragma once

#include "orderreader.h"
#include "orderrepository.h"

class FileOrderRepository : public OrderRepository, public OrderReader {
public:
    FileOrderRepository(const QString& orderFilePathToUse, const QString& orderDetailFilePathToUse);

    QString createNextOrderId(const QDate& orderDate) override;
    bool saveOrder(const Order& order, QString& problemDescription) override;
    bool replaceOrder(const Order& order, QString& problemDescription) override;

    bool loadOrdersWithStatus(const QString& status, vector<Order>& orders, QString& problemDescription) override;
    bool loadAllOrders(vector<Order>& orders, QString& problemDescription) override;
    bool loadOrderDetails(const QString& orderId, vector<OrderDetailRecord>& details, QString& problemDescription) override;

private:
    QString orderFilePath;
    QString orderDetailFilePath;
};
