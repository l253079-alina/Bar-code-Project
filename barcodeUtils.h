
#pragma once

#include <QImage> 
#include <QString>
#include <optional>
using namespace std;

optional<QString> decodeBarcode(const QImage &src);
QString toSixDigits(const QString &text); 
QImage makeBarcodeImage(const QString &sixDigits);
