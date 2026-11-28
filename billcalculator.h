#pragma once

#include "bill.h"
#include "order.h"
#include <QString>

struct BillTotals 
{
    double withoutTax = 0.0;
    double tax = 0.0;
    double discount = 0.0;
    double beforeExtraDiscounts = 0.0;
    double seasonalDiscount = 0.0;
    double specialDiscount = 0.0;
    double payable = 0.0;
};

class BillCalculator {
public:
    static BillTotals calculateTotals(const vector<OrderItem>& orderItems, double seasonalDiscountPercent = 0.0,
  double specialDiscountPercent = 0.0);
    static Bill createBill(const Order& order, const QString& billId, const BillTotals& totals,
  double amountReceived, const QString& billDate, const QString& billTime);
    static double roundMoney(double amount);
};
