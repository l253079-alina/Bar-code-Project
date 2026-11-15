#include "fileorderrepository.h"

#include "orderrecordformatter.h"
#include "textfilelines.h"

static const int orderIdLength = 8;
static const int orderSequenceLength = 6;
static const int orderDayLength = 2;

FileOrderRepository::FileOrderRepository(const QString& orderFilePathToUse, const QString& orderDetailFilePathToUse)
    : orderFilePath(orderFilePathToUse), orderDetailFilePath(orderDetailFilePathToUse)
{
}

QString FileOrderRepository::createNextOrderId(const QDate& orderDate)
{
    const int highestSequenceNumber =
        TextFileLines::highestIdNumber(orderFilePath, orderIdLength, orderSequenceLength);
    const int nextSequenceNumber = highestSequenceNumber + 1;
    return QString("%1%2")
        .arg(nextSequenceNumber, orderSequenceLength, 10, QChar('0'))
        .arg(orderDate.day(), orderDayLength, 10, QChar('0'));
}

bool FileOrderRepository::saveOrder(const Order& order, QString& problemDescription)
{
    const QStringList orderLines = QStringList() << OrderRecordFormatter::orderLine(order);
    if (!TextFileLines::appendLines(orderFilePath, orderLines, problemDescription))
        return false;

    return TextFileLines::appendLines(orderDetailFilePath, OrderRecordFormatter::orderDetailLines(order),
                                      problemDescription);
}
