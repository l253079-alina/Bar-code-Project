#pragma once

#include <QString>
#include <QStringList>

class FileFieldFormatter {
public:
    static QString text(const QString& value);
    static QString money(double amount);
    static QString quantity(double quantityValue);
    static QString lineWithEndMarker(const QStringList& fields);
};
