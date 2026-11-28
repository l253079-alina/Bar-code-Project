#include "billcalculator.h"

#include "orderconstants.h"

double BillCalculator::roundMoney(double amount)
{
    return floor(amount * 100.0 + 0.5) / 100.0;
}

BillTotals BillCalculator::calculateTotals(const vector<OrderItem>& orderItems, double seasonalDiscountPercent,
double specialDiscountPercent)
{
    BillTotals totals;
    for (const OrderItem& item : orderItems) {
        totals.withoutTax += item.product.unitPrice * item.quantity;
        totals.tax += item.product.taxAmountPerUnit() * item.quantity;
        totals.discount += item.product.discountAmountPerUnit() * item.quantity;
    }
    totals.withoutTax = roundMoney(totals.withoutTax);
    totals.tax = roundMoney(totals.tax);
    totals.discount = roundMoney(totals.discount);

    totals.beforeExtraDiscounts = roundMoney(totals.withoutTax - totals.discount + totals.tax);
    totals.seasonalDiscount = roundMoney(totals.beforeExtraDiscounts * seasonalDiscountPercent / 100.0);
    const double amountAfterSeasonalDiscount = totals.beforeExtraDiscounts - totals.seasonalDiscount;
    totals.specialDiscount = roundMoney(amountAfterSeasonalDiscount * specialDiscountPercent / 100.0);
    totals.payable = roundMoney(amountAfterSeasonalDiscount - totals.specialDiscount);
    return totals;
}

Bill BillCalculator::createBill(const Order& order, const QString& billId, const BillTotals& totals,
double amountReceived, const QString& billDate, const QString& billTime)
{
    Bill bill;
    bill.billId = billId;
    bill.customerName = order.customerName;
    bill.orderId = order.orderId;
    bill.billDate = billDate;
    bill.billTime = billTime;
    bill.paymentStatus = order.status;

    for (const OrderItem& item : order.items) {
        BillLine line;
        line.productAcronym = item.product.acronym;
        line.productName = item.product.name;
        line.unitPriceWithoutTax = item.product.unitPrice;
        line.discountPerUnit = item.product.discountAmountPerUnit();
        line.taxPerUnit = item.product.taxAmountPerUnit();
        line.quantity = item.quantity;
        bill.lines.push_back(line);
    }

    bill.totalWithoutTax = totals.withoutTax;
    bill.totalTax = totals.tax;
    bill.totalDiscount = totals.discount;
    bill.seasonalDiscount = totals.seasonalDiscount;
    bill.specialDiscount = totals.specialDiscount;
    bill.totalPayable = totals.payable;

    if (order.status == OrderConstants::statusPaid) {
        bill.amountReceived = amountReceived;
        if (amountReceived > totals.payable)
            bill.balanceReturned = roundMoney(amountReceived - totals.payable);
    }
    return bill;
}
