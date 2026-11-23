#pragma once

#include "categorycatalog.h"
#include "productcatalog.h"

#include <QImage>
#include <QWidget>

class QComboBox;
class QLabel;
class QLineEdit;

class BarcodeGeneratorScreen : public QWidget {
public:
    BarcodeGeneratorScreen(ProductCatalog& catalog, CategoryCatalog& categories, const QString& dataFolderToUse,
        QWidget* parent = nullptr);
    void reloadData();

private:
    ProductCatalog& productCatalog;
    CategoryCatalog& categoryCatalog;
    QString dataFolderPath;
    QImage generatedBarcode;

    QComboBox* productComboBox = nullptr;
    QComboBox* categoryComboBox = nullptr;
    QLineEdit* numberLineEdit = nullptr;
    QLabel* barcodeView = nullptr;
    QLabel* messageLabel = nullptr;

    void useSelectedProduct();
    void useSelectedCategory();
    void generateBarcode();
    void saveBarcode();
    QString chooseSavePath() const;
};