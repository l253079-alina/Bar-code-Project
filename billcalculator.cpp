#include "billcalculator.h"

BillTotals BillCalculator::calculateTotals(const vector<OrderItem>& orderItems)
{
    BillTotals totals;
    for (const OrderItem& item : orderItems) {
        totals.withoutTax += item.product.unitPrice * item.quantity;
        totals.tax += item.product.taxPerUnit * item.quantity;
        totals.discount += item.product.discountPerUnit * item.quantity;
    }
    totals.payable = totals.withoutTax - totals.discount + totals.tax;
    return totals;
}

Bill BillCalculator::createBill(const Order& order, const QString& billId)
{
    Bill bill;
    bill.billId = billId;
    bill.customerName = order.customerName;
    bill.orderId = order.orderId;
    bill.orderDate = order.orderDate;
    bill.orderTime = order.orderTime;
    bill.paymentStatus = order.status;

    for (const OrderItem& item : order.items) {
        BillLine line;
        line.productAcronym = item.product.acronym;
        line.productName = item.product.name;
        line.unitPriceWithoutTax = item.product.unitPrice;
        line.discountPerUnit = item.product.discountPerUnit;
        line.taxPerUnit = item.product.taxPerUnit;
        line.quantity = item.quantity;
        bill.lines.push_back(line);
    }

    const BillTotals totals = calculateTotals(order.items);
    bill.totalWithoutTax = totals.withoutTax;
    bill.totalTax = totals.tax;
    bill.totalDiscount = totals.discount;
    bill.totalPayable = totals.payable;
    return bill;
}
