#include "textfilelines.h"

#include <QFile>
#include <QSaveFile>
#include <QTextStream>

QStringList TextFileLines::readNonEmptyLines(const QString& filePath)
{
    QStringList lines;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return lines;

    QTextStream fileStream(&file);
    while (!fileStream.atEnd()) {
        const QString line = fileStream.readLine().trimmed();
        if (!line.isEmpty())
            lines.append(line);
    }
    return lines;
}

bool TextFileLines::appendLines(const QString& filePath, const QStringList& lines, QString& problemDescription)
{
    QFile file(filePath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        problemDescription = "Could not write to " + filePath;
        return false;
    }

    QTextStream fileStream(&file);
    for (const QString& line : lines)
        fileStream << line << Qt::endl;
    return true;
}

bool TextFileLines::writeAllLines(const QString& filePath, const QStringList& lines, QString& problemDescription)
{
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        problemDescription = "Could not write to " + filePath;
        return false;
    }

    QTextStream fileStream(&file);
    for (const QString& line : lines)
        fileStream << line << Qt::endl;
    fileStream.flush();

    if (!file.commit()) {
        problemDescription = "Could not save " + filePath;
        return false;
    }
    return true;
}

static bool containsOnlyDigits(const QString& text)
{
    if (text.isEmpty())
        return false;
    for (const QChar character : text) {
        if (!character.isDigit())
            return false;
    }
    return true;
}

int TextFileLines::highestIdNumber(const QString& filePath, int idLength, int numberLength)
{
    int highestNumber = 0;
    const QStringList lines = readNonEmptyLines(filePath);
    for (const QString& line : lines) {
        const QString firstField = line.section(' ', 0, 0);
        if (firstField.size() != idLength || !containsOnlyDigits(firstField))
            continue;

        const int numberInId = firstField.left(numberLength).toInt();
        if (numberInId > highestNumber)
            highestNumber = numberInId;
    }
    return highestNumber;
}
