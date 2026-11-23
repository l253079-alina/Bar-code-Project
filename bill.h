#pragma once
#include "standardnames.h"
#include <QString>
struct BillLine 
{
    QString productAcronym;
    QString productName;
    double unitPriceWithoutTax = 0.0;
    double discountPerUnit = 0.0;
    double taxPerUnit = 0.0;
    double quantity = 0.0;
};
struct Bill 
{
    QString billId;
    QString customerName;
    QString orderId;
    QString billDate;
    QString billTime;
    vector<BillLine> lines;
    double totalWithoutTax = 0.0;
    double totalTax = 0.0;
    double totalDiscount = 0.0;
    double seasonalDiscount = 0.0;
    double specialDiscount = 0.0;
    double totalPayable = 0.0;
    double amountReceived = 0.0;
    double balanceReturned = 0.0;
    QString paymentStatus;
};
