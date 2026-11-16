#pragma once
#include "bill.h"
#include "order.h"
#include <QString>

struct BillTotals {
    double withoutTax = 0.0;
    double tax = 0.0;
    double discount = 0.0;
    double payable = 0.0;
};
class BillCalculator {
public:
    static BillTotals calculateTotals(const vector<OrderItem>& orderItems);
    static Bill createBill(const Order& order, const QString& billId);
};
