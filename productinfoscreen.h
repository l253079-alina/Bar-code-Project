#pragma once

#include "productcatalog.h"

#include <QWidget>

class QTextEdit;
class ScanPanel;

class ProductInfoScreen : public QWidget {
public:
    explicit ProductInfoScreen(ProductCatalog& catalog, QWidget* parent = nullptr);
    void stopScanning();
    void showProductForBarcode(const QString& scannedBarcode);

private:
    ProductCatalog& productCatalog;
    ScanPanel* scanPanel = nullptr;
    QTextEdit* descriptionView = nullptr;
};
