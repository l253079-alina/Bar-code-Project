#pragma once
#include <QString>

struct Product {
    QString productId;
    QString barcode;
    QString categoryId;
    QString acronym;
    QString name;
    QString measurementUnit;
    double unitPrice = 0.0;
    double taxPerUnit = 0.0;
    double discountPerUnit = 0.0;
    double availableQuantity = 0.0;
    QString barcodeFilePath;
};