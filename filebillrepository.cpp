#include "filebillrepository.h"
#include "billformatter.h"
#include "textfilelines.h"

static const int billIdLength = 9;
FileBillRepository::FileBillRepository(const QString& billFilePathToUse): billFilePath(billFilePathToUse)
{}
QString FileBillRepository::createNextBillId()
{
    const int highestBillNumber = TextFileLines::highestIdNumber(billFilePath, billIdLength, billIdLength);
    return QString("%1").arg(highestBillNumber + 1, billIdLength, 10, QChar('0'));
}

bool FileBillRepository::saveBill(const Bill& bill, QString& problemDescription)
{
    const QStringList billLines = QStringList() << BillFormatter::fileLine(bill);
    return TextFileLines::appendLines(billFilePath, billLines, problemDescription);
}
