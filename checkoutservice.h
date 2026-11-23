#pragma once
#include "bill.h"
#include "billrepository.h"
#include "existingorderinfo.h"
#include "orderitem.h"
#include "orderrepository.h"
#include "productcatalog.h"
#include "seasoncatalog.h"
#include "standardnames.h"
#include <QDate>
#include <QString>

struct CheckoutDetails 
{
    QString customerName;
    QString customerContact;
    QString orderStatus;
    QString paymentMode;
    QString paymentMerchantName;
    double amountReceived = 0.0;
    double specialDiscountPercent = 0.0;
};

class CheckoutService 
{
public:
    CheckoutService(OrderRepository& orderRepositoryToUse, BillRepository& billRepositoryToUse, ProductCatalog& productCatalogToUse, SeasonCatalog& seasonCatalogToUse);
    double previewSeasonalPercent(const optional<ExistingOrderInfo>& existingOrder);
    bool completeCheckout(const vector<OrderItem>& orderItems, const CheckoutDetails& checkoutDetails, const optional<ExistingOrderInfo>& existingOrder, Bill& createdBill,
 QString& problemDescription);

private:
    OrderRepository& orderRepository;
    BillRepository& billRepository;
    ProductCatalog& productCatalog;
    SeasonCatalog& seasonCatalog;
    bool findPreviousBill(const optional<ExistingOrderInfo>& existingOrder, Bill& previousBill);
};
