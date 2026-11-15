#pragma once

#include <QString>
#include <QStringList>

class TextFileLines {
public:
    static QStringList readNonEmptyLines(const QString& filePath);
    static bool appendLines(const QString& filePath, const QStringList& lines, QString& problemDescription);
    static int highestIdNumber(const QString& filePath, int idLength, int numberLength);
};
