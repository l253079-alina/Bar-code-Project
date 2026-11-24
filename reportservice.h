#pragma once

#include "billrepository.h"
#include "orderreader.h"
#include "productcatalog.h"
#include "standardnames.h"
#include <QDate>
#include <QString>
#include <QStringList>

struct ReportTable 
{
    QStringList headers;
    vector<QStringList> rows;
    QString summary;
};

class ReportService 
{
public:
    ReportService(OrderReader& readerToUse, BillRepository& billRepositoryToUse, ProductCatalog& catalogToUse);
    bool incompleteOrdersReport(ReportTable& table, QString& problemDescription);
    bool completedOrdersReport(ReportTable& table, QString& problemDescription);
    bool cashSalesReport(const QDate& fromDate, const QDate& toDate, ReportTable& table, QString& problemDescription);
    bool allSalesReport(const QDate& fromDate, const QDate& toDate, ReportTable& table, QString& problemDescription);
    bool topSoldItemsReport(ReportTable& table, QString& problemDescription);
    bool topRemainingItemsReport(ReportTable& table, QString& problemDescription);

private:
    OrderReader& orderReader;
    BillRepository& billRepository;
    ProductCatalog& productCatalog;
    double totalForOrder(const Order& order, const map<QString, Bill>& latestBillByOrderId, QString& problemDescription);
    bool loadBillsByOrder(map<QString, Bill>& latestBillByOrderId, QString& problemDescription);
};
