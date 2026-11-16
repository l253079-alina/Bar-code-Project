#include "billformatter.h"
#include "filefieldformatter.h"
#include <QStringList>

static QString leftColumn(const QString& text, int columnWidth)
{
    return text.leftJustified(columnWidth, QChar(' '), true);
}

static QString rightColumn(const QString& text, int columnWidth)
{
    return text.rightJustified(columnWidth, QChar(' '));
}

static double lineTotal(const BillLine& line)
{
    const double unitPriceToPay = line.unitPriceWithoutTax - line.discountPerUnit + line.taxPerUnit;
    return unitPriceToPay * line.quantity;
}

QString BillFormatter::fileLine(const Bill& bill)
{
    QStringList acronyms;
    QStringList unitPrices;
    QStringList discounts;
    QStringList quantities;
    for (const BillLine& line : bill.lines) {
        acronyms << FileFieldFormatter::text(line.productAcronym);
        unitPrices << FileFieldFormatter::money(line.unitPriceWithoutTax);
        discounts << FileFieldFormatter::money(line.discountPerUnit);
        quantities << FileFieldFormatter::quantity(line.quantity);
    }

    QStringList fields;
    fields << bill.billId
           << FileFieldFormatter::text(bill.customerName)
           << bill.orderId
           << acronyms.join(QChar(','))
           << unitPrices.join(QChar(','))
           << discounts.join(QChar(','))
           << quantities.join(QChar(','))
           << FileFieldFormatter::money(bill.totalWithoutTax)
           << FileFieldFormatter::money(bill.totalTax)
           << FileFieldFormatter::money(bill.totalDiscount)
           << FileFieldFormatter::money(bill.totalPayable)
           << FileFieldFormatter::text(bill.paymentStatus);
    return FileFieldFormatter::lineWithEndMarker(fields);
}

QString BillFormatter::screenText(const Bill& bill)
{
    const QString separator(78, QChar('-'));
    QStringList lines;
    lines << QString("BILL %1").arg(bill.billId);
    lines << QString("Order: %1    Date: %2    Time: %3").arg(bill.orderId, bill.orderDate, bill.orderTime);
    lines << QString("Customer: %1").arg(bill.customerName);
    lines << QString("Status: %1").arg(bill.paymentStatus);
    lines << separator;
    lines << leftColumn("Item", 9) + leftColumn("Name", 18) + rightColumn("Qty", 8) + rightColumn("Price", 10)
                 + rightColumn("Discount", 10) + rightColumn("Tax", 8) + rightColumn("Total", 15);
    lines << separator;

    for (const BillLine& line : bill.lines) {
        lines << leftColumn(line.productAcronym, 9)
                     + leftColumn(line.productName, 18)
                     + rightColumn(FileFieldFormatter::quantity(line.quantity), 8)
                     + rightColumn(FileFieldFormatter::money(line.unitPriceWithoutTax), 10)
                     + rightColumn(FileFieldFormatter::money(line.discountPerUnit), 10)
                     + rightColumn(FileFieldFormatter::money(line.taxPerUnit), 8)
                     + rightColumn(FileFieldFormatter::money(lineTotal(line)), 15);
    }

    lines << separator;
    lines << rightColumn("Total without tax:", 50) + rightColumn(FileFieldFormatter::money(bill.totalWithoutTax), 28);
    lines << rightColumn("Total discount:", 50) + rightColumn(FileFieldFormatter::money(bill.totalDiscount), 28);
    lines << rightColumn("Total tax:", 50) + rightColumn(FileFieldFormatter::money(bill.totalTax), 28);
    lines << rightColumn("Total payable (PKR):", 50) + rightColumn(FileFieldFormatter::money(bill.totalPayable), 28);
    return lines.join(QChar('\n'));
}
