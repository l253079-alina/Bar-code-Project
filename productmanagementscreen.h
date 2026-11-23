#pragma once
#include "categorycatalog.h"
#include "product.h"
#include "productcatalog.h"
#include <QWidget>

class QLabel;
class QTableWidget;

class ProductManagementScreen : public QWidget {
public:
    ProductManagementScreen(ProductCatalog& products, CategoryCatalog& categories, const QString& dataFolderToUse,
        QWidget* parent = nullptr);
    void refresh();

private:
    ProductCatalog& productCatalog;
    CategoryCatalog& categoryCatalog;
    QString dataFolderPath;

    QTableWidget* productTable = nullptr;
    QLabel* messageLabel = nullptr;

    void addProduct();
    void editSelectedProduct();
    void updateInventoryOfSelectedProduct();
    QString saveBarcodeImage(const Product& product) const;
    optional<Product> selectedProduct() const;
};