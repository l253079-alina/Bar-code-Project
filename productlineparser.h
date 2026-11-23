#pragma once
#include "product.h"
#include <QString>
#include <QStringList>

class ProductLineParser {
public:
    static bool parseLine(const QString& rawLine, Product& product);

private:
    static bool isMeasurementUnit(const QString& token);
    static bool areAllNumbers(const QStringList& tokens, int firstTokenIndex, int tokenCount);
    static bool hasCategoryToken(const QStringList& tokens);
};