#include "billrecordparser.h"
#include "filefieldformatter.h"

static const int billFieldCount = 14;
static const int billDetailFieldCount = 5;
static bool readNumbers(const QStringList& fields, const vector<int>& fieldIndexes, vector<double>& numbers)
{
    numbers.clear();
    for (const int fieldIndex : fieldIndexes) {
        bool fieldIsNumber = false;
        const double number = fields[fieldIndex].toDouble(&fieldIsNumber);
        if (!fieldIsNumber)
            return false;
        numbers.push_back(number);
    }
    return true;
}
bool BillRecordParser::parseBillLine(const QString& rawLine, Bill& bill)
{
    const QStringList fields = FileFieldFormatter::fieldsOfLine(rawLine);
    if (fields.size() != billFieldCount)
        return false;

    vector<double> numbers;
    if (!readNumbers(fields, { 3, 4, 5, 6, 7, 8, 10, 11 }, numbers))
        return false;

    bill.billId = fields[0];
    bill.customerName = FileFieldFormatter::restoreText(fields[1]);
    bill.orderId = fields[2];
    bill.totalWithoutTax = numbers[0];
    bill.totalTax = numbers[1];
    bill.totalDiscount = numbers[2];
    bill.totalPayable = numbers[3];
    bill.amountReceived = numbers[4];
    bill.balanceReturned = numbers[5];
    bill.paymentStatus = fields[9];
    bill.seasonalDiscount = numbers[6];
    bill.specialDiscount = numbers[7];
    bill.billDate = fields[12];
    bill.billTime = fields[13];
    return true;
}

bool BillRecordParser::parseBillDetailLine(const QString& rawLine, QString& billId, BillLine& billLine)
{
    const QStringList fields = FileFieldFormatter::fieldsOfLine(rawLine);
    if (fields.size() != billDetailFieldCount)
        return false;

    vector<double> numbers;
    if (!readNumbers(fields, { 2, 3, 4 }, numbers))
        return false;

    const double quantity = numbers[1];
    const double discountOnProduct = numbers[2];

    billId = fields[0];
    billLine.productAcronym = FileFieldFormatter::restoreText(fields[1]);
    billLine.productName = billLine.productAcronym;
    billLine.unitPriceWithoutTax = numbers[0];
    billLine.quantity = quantity;
    billLine.discountPerUnit = 0.0;
    if (quantity > 0.0)
        billLine.discountPerUnit = discountOnProduct / quantity;
    billLine.taxPerUnit = 0.0;
    return true;
}
