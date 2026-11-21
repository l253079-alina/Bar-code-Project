#pragma once
#include "bill.h"
#include "billrepository.h"
#include "orderitem.h"
#include "orderrepository.h"
#include "productcatalog.h"
#include "standardnames.h"
#include <QString>
struct CheckoutDetails 
{
    QString customerName;
    QString customerContact;
    QString orderStatus;
    QString paymentMode;
    QString paymentMerchantName;
};
class CheckoutService 
{
public:
    CheckoutService(OrderRepository& orderRepositoryToUse, BillRepository& billRepositoryToUse,ProductCatalog& productCatalogToUse);
    bool completeCheckout(const vector<OrderItem>& orderItems, const CheckoutDetails& checkoutDetails,Bill& createdBill, QString& problemDescription);
private:
    OrderRepository& orderRepository;
    BillRepository& billRepository;
    ProductCatalog& productCatalog;
};
