#pragma once

#include "billrepository.h"
#include "existingorderinfo.h"
#include "orderitem.h"
#include "orderreader.h"
#include "orderrepository.h"
#include "productcatalog.h"
#include "standardnames.h"
#include <QDate>
#include <QString>
#include <QStringList>

struct HeldOrderSummary 
{
    QString orderId;
    QString orderDate;
    QString orderTime;
    QString customerName;
    QString status;
    QString itemSummary;
    double totalPayable = 0.0;
    bool requiresApproval = false;
};
struct HeldOrderContents 
{
    ExistingOrderInfo orderInfo;
    vector<OrderItem> items;
    QString warning;
};

class HeldOrderService 
{
public:
    HeldOrderService(OrderRepository& repositoryToUse, OrderReader& readerToUse, ProductCatalog& catalogToUse, BillRepository& billRepositoryToUse);
    bool holdOrder(const vector<OrderItem>& orderItems, const QString& customerName,const optional<ExistingOrderInfo>& existingOrder, QString& heldOrderId,
  QString& problemDescription);
    bool findHeldOrders(vector<HeldOrderSummary>& summaries, QString& problemDescription);
    bool findEditableOrders(vector<HeldOrderSummary>& summaries, QString& problemDescription);
    bool openOrder(const QString& orderId, HeldOrderContents& contents, QString& problemDescription);

private:
    OrderRepository& orderRepository;
    OrderReader& orderReader;
    ProductCatalog& productCatalog;
    BillRepository& billRepository;
    void buildItems(const vector<OrderDetailRecord>& details, vector<OrderItem>& items, QStringList& missingProducts) const;
    bool buildSummaries(const vector<Order>& orders, vector<HeldOrderSummary>& summaries, QString& problemDescription);
    map<QString, QDate> loadPaymentDates();
    static QDate paymentDateOf(const Order& order, const map<QString, QDate>& paymentDateByOrderId);
    static bool isWithinEditWindow(const QDate& paymentDate);
};
