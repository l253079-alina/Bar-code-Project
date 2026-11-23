#pragma once

#include "categorycatalog.h"
#include "product.h"
#include "productcatalog.h"
#include "standardnames.h"
#include <QDialog>

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;

class ProductDialog : public QDialog {
public:
    ProductDialog(const ProductCatalog& catalog, const CategoryCatalog& categories,
        const optional<Product>& productToEdit, QWidget* parent = nullptr);
    Product product() const;
    void accept() override;

private:
    const ProductCatalog& productCatalog;
    const CategoryCatalog& categoryCatalog;
    optional<Product> originalProduct;
    QString pendingBarcode;

    QComboBox* categoryComboBox = nullptr;
    QLineEdit* nameLineEdit = nullptr;
    QLineEdit* acronymLineEdit = nullptr;
    QComboBox* measurementUnitComboBox = nullptr;
    QDoubleSpinBox* unitPriceSpinBox = nullptr;
    QDoubleSpinBox* taxSpinBox = nullptr;
    QDoubleSpinBox* discountSpinBox = nullptr;
    QDoubleSpinBox* quantitySpinBox = nullptr;
    QLabel* barcodeLabel = nullptr;

    void updatePendingBarcode();
    QString selectedCategoryId() const;
};