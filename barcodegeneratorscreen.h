#pragma once

#include "productcatalog.h"

#include <QImage>
#include <QWidget>

class QComboBox;
class QLabel;
class QLineEdit;

class BarcodeGeneratorScreen : public QWidget {
public:
    BarcodeGeneratorScreen(ProductCatalog& catalog, const QString& dataFolderToUse, QWidget* parent = nullptr);
    void reloadProducts();

private:
    ProductCatalog& productCatalog;
    QString dataFolderPath;
    QImage generatedBarcode;

    QComboBox* productComboBox = nullptr;
    QLineEdit* numberLineEdit = nullptr;
    QLabel* barcodeView = nullptr;
    QLabel* messageLabel = nullptr;

    void useSelectedProduct();
    void generateBarcode();
    void saveBarcode();
    QString chooseSavePath() const;
};