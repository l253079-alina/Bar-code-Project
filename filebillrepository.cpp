#include "filebillrepository.h"
#include "billformatter.h"
#include "billrecordparser.h"
#include "textfilelines.h"
static const int billIdLength = 9;

FileBillRepository::FileBillRepository(const QString& billFilePathToUse, const QString& billDetailFilePathToUse) : billFilePath(billFilePathToUse), billDetailFilePath(billDetailFilePathToUse)
{}

QString FileBillRepository::createNextBillId()
{
    const int highestBillNumber = TextFileLines::highestIdNumber(billFilePath, billIdLength, billIdLength);
    return QString("%1").arg(highestBillNumber + 1, billIdLength, 10, QChar('0'));
}

bool FileBillRepository::saveBill(const Bill& bill, QString& problemDescription)
{
    const QStringList billLines = QStringList() << BillFormatter::fileLine(bill);
    if (!TextFileLines::appendLines(billFilePath, billLines, problemDescription))
        return false;
    return TextFileLines::appendLines(billDetailFilePath, BillFormatter::detailLines(bill), problemDescription);
}

bool FileBillRepository::replaceBill(const Bill& bill, QString& problemDescription)
{
    QStringList billLines = TextFileLines::readNonEmptyLines(billFilePath);
    bool billWasFound = false;
    for (QString& billLine : billLines) {
        if (billLine.section(' ', 0, 0) == bill.billId) {
            billLine = BillFormatter::fileLine(bill);
            billWasFound = true;
        }
    }
    if (!billWasFound) {
        problemDescription = "Bill " + bill.billId + " was not found in " + billFilePath;
        return false;
    }
    if (!TextFileLines::writeAllLines(billFilePath, billLines, problemDescription))
        return false;

    QStringList keptDetailLines;
    for (const QString& detailLine : TextFileLines::readNonEmptyLines(billDetailFilePath)) {
        if (detailLine.section(' ', 0, 0) != bill.billId)
            keptDetailLines.append(detailLine);
    }
    keptDetailLines.append(BillFormatter::detailLines(bill));
    return TextFileLines::writeAllLines(billDetailFilePath, keptDetailLines, problemDescription);
}

bool FileBillRepository::loadBills(vector<Bill>& bills, QString& problemDescription)
{
    bills.clear();
    problemDescription.clear();
    map<QString, size_t> positionByBillId;
    for (const QString& billLine : TextFileLines::readNonEmptyLines(billFilePath)) {
        Bill bill;
        if (BillRecordParser::parseBillLine(billLine, bill)) {
            positionByBillId[bill.billId] = bills.size();
            bills.push_back(bill);
        }
    }

    for (const QString& detailLine : TextFileLines::readNonEmptyLines(billDetailFilePath)) {
        QString billId;
        BillLine billLine;
        if (!BillRecordParser::parseBillDetailLine(detailLine, billId, billLine))
            continue;
        const auto foundPosition = positionByBillId.find(billId);
        if (foundPosition != positionByBillId.end())
            bills[foundPosition->second].lines.push_back(billLine);
    }
    return true;
}

bool FileBillRepository::findBillByOrderId(const QString& orderId, Bill& bill)
{
    vector<Bill> bills;
    QString ignoredProblem;
    loadBills(bills, ignoredProblem);
    for (size_t position = bills.size(); position > 0; --position) {
        if (bills[position - 1].orderId == orderId) {
            bill = bills[position - 1];
            return true;
        }
    }
    return false;
}
