#include "productlineparser.h"

#include "orderconstants.h"

bool ProductLineParser::isMeasurementUnit(const QString& token){
    const QString lowerCaseToken = token.toLower();
    return lowerCaseToken == OrderConstants::measurementUnitCount
        || lowerCaseToken == OrderConstants::measurementUnitWeight;
}

bool ProductLineParser::areAllNumbers(const QStringList& tokens, int firstTokenIndex, int tokenCount){
    for (int offset = 0; offset < tokenCount; ++offset) {
        bool conversionWorked = false;
        tokens[firstTokenIndex + offset].toDouble(&conversionWorked);
        if (!conversionWorked)
            return false;
    }
    return true;
}

bool ProductLineParser::parseLine(const QString& rawLine, Product& product){
    QString lineWithoutEndMarker = rawLine.trimmed();
    if (lineWithoutEndMarker.endsWith('#'))
        lineWithoutEndMarker.chop(1);

    const QStringList tokens = lineWithoutEndMarker.trimmed().split(' ', Qt::SkipEmptyParts);

    const int leadingTokenCount = 3;
    const int numberTokenCount = 4;
    const int minimumTokenCount = leadingTokenCount + 1 + 1 + numberTokenCount + 1;
    if (tokens.size() < minimumTokenCount)
        return false;

    int unitTokenIndex = -1;
    for (int index = leadingTokenCount + 1; index + numberTokenCount + 1 < tokens.size(); ++index) {
        if (isMeasurementUnit(tokens[index]) && areAllNumbers(tokens, index + 1, numberTokenCount)) {
            unitTokenIndex = index;
            break;
        }
    }
    if (unitTokenIndex < 0)
        return false;

    product.productId = tokens[0];
    product.barcode = tokens[1];
    product.acronym = tokens[2];
    product.name = tokens.mid(leadingTokenCount, unitTokenIndex - leadingTokenCount).join(' ');
    product.measurementUnit = tokens[unitTokenIndex].toLower();
    product.unitPrice = tokens[unitTokenIndex + 1].toDouble();
    product.taxPerUnit = tokens[unitTokenIndex + 2].toDouble();
    product.discountPerUnit = tokens[unitTokenIndex + 3].toDouble();
    product.availableQuantity = tokens[unitTokenIndex + 4].toDouble();
    product.barcodeFilePath = tokens.mid(unitTokenIndex + numberTokenCount + 1).join(' ');
    return true;
}