#include "filefieldformatter.h"

#include <QRegularExpression>

QString FileFieldFormatter::text(const QString& value)
{
    QString cleanedValue = value.trimmed();
    cleanedValue.replace(QChar('#'), QChar('_'));
    cleanedValue.replace(QRegularExpression("\\s+"), "_");
    if (cleanedValue.isEmpty())
        return "-";
    return cleanedValue;
}

QString FileFieldFormatter::money(double amount)
{
    return QString::number(amount, 'f', 2);
}

QString FileFieldFormatter::quantity(double quantityValue)
{
    return QString::number(quantityValue, 'g', 10);
}

QString FileFieldFormatter::lineWithEndMarker(const QStringList& fields)
{
    return fields.join(QChar(' ')) + " #";
}
