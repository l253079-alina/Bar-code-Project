#include "productdialog.h"
#include "orderconstants.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QVBoxLayout>

static const double largestAmount = 100000000.0;
static const double quantityTolerance = 0.0000001;

ProductDialog::ProductDialog(const ProductCatalog& catalog, const CategoryCatalog& categories,
    const optional<Product>& productToEdit, QWidget* parent)
    : QDialog(parent), productCatalog(catalog), categoryCatalog(categories), originalProduct(productToEdit){
    if (originalProduct.has_value())
        setWindowTitle("Edit product");
    else
        setWindowTitle("Add product");

    categoryComboBox = new QComboBox;
    for (const Category& category : categoryCatalog.allCategories())
        categoryComboBox->addItem(category.categoryId + " - " + category.name, category.categoryId);

    nameLineEdit = new QLineEdit;
    acronymLineEdit = new QLineEdit;
    acronymLineEdit->setPlaceholderText("Short code without spaces");

    measurementUnitComboBox = new QComboBox;
    measurementUnitComboBox->addItems({ OrderConstants::measurementUnitCount, OrderConstants::measurementUnitWeight });

    unitPriceSpinBox = new QDoubleSpinBox;
    taxSpinBox = new QDoubleSpinBox;
    discountSpinBox = new QDoubleSpinBox;
    quantitySpinBox = new QDoubleSpinBox;
    for (QDoubleSpinBox* amountSpinBox : { unitPriceSpinBox, taxSpinBox, discountSpinBox }) {
        amountSpinBox->setDecimals(2);
        amountSpinBox->setRange(0.0, largestAmount);
    }
    quantitySpinBox->setDecimals(3);
    quantitySpinBox->setRange(0.0, largestAmount);

    barcodeLabel = new QLabel;
    barcodeLabel->setStyleSheet("font-weight: bold;");

    if (originalProduct.has_value()) {
        int categoryIndex = categoryComboBox->findData(originalProduct->categoryId);
        if (categoryIndex < 0) {
            categoryComboBox->addItem(originalProduct->categoryId + " - (not configured)", originalProduct->categoryId);
            categoryIndex = categoryComboBox->count() - 1;
        }
        categoryComboBox->setCurrentIndex(categoryIndex);
        categoryComboBox->setEnabled(false);

        nameLineEdit->setText(originalProduct->name);
        acronymLineEdit->setText(originalProduct->acronym);
        measurementUnitComboBox->setCurrentText(originalProduct->measurementUnit);
        unitPriceSpinBox->setValue(originalProduct->unitPrice);
        taxSpinBox->setValue(originalProduct->taxPerUnit);
        discountSpinBox->setValue(originalProduct->discountPerUnit);
        quantitySpinBox->setValue(originalProduct->availableQuantity);
        pendingBarcode = originalProduct->barcode;
        barcodeLabel->setText(pendingBarcode);
    }
    else {
        updatePendingBarcode();
        connect(categoryComboBox, &QComboBox::currentIndexChanged, this, [this] { updatePendingBarcode(); });
    }

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("Category", categoryComboBox);
    formLayout->addRow("Product name", nameLineEdit);
    formLayout->addRow("Acronym", acronymLineEdit);
    formLayout->addRow("Measurement unit", measurementUnitComboBox);
    formLayout->addRow("Unit price (PKR)", unitPriceSpinBox);
    formLayout->addRow("Tax per unit (PKR)", taxSpinBox);
    formLayout->addRow("Discount per unit (PKR)", discountSpinBox);
    formLayout->addRow("Available quantity", quantitySpinBox);
    formLayout->addRow("Barcode", barcodeLabel);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(formLayout);
    layout->addWidget(buttonBox);
}

QString ProductDialog::selectedCategoryId() const{
    return categoryComboBox->currentData().toString();
}

void ProductDialog::updatePendingBarcode(){
    pendingBarcode.clear();
    const QString chosenCategoryId = selectedCategoryId();
    if (chosenCategoryId.isEmpty()) {
        barcodeLabel->setText("Add a category first");
        return;
    }

    QString problemDescription;
    if (!productCatalog.nextBarcodeForCategory(chosenCategoryId, pendingBarcode, problemDescription)) {
        barcodeLabel->setText(problemDescription);
        pendingBarcode.clear();
        return;
    }
    barcodeLabel->setText(pendingBarcode + " (created automatically)");
}

Product ProductDialog::product() const{
    Product resultingProduct;
    if (originalProduct.has_value()) {
        resultingProduct = *originalProduct;
    }
    else {
        resultingProduct.productId = productCatalog.createNextProductId();
        resultingProduct.barcode = pendingBarcode;
        resultingProduct.categoryId = selectedCategoryId();
        resultingProduct.barcodeFilePath = "Barcodes/" + pendingBarcode + ".png";
    }
    resultingProduct.name = nameLineEdit->text().simplified();
    resultingProduct.acronym = acronymLineEdit->text().trimmed();
    resultingProduct.measurementUnit = measurementUnitComboBox->currentText();
    resultingProduct.unitPrice = unitPriceSpinBox->value();
    resultingProduct.taxPerUnit = taxSpinBox->value();
    resultingProduct.discountPerUnit = discountSpinBox->value();
    resultingProduct.availableQuantity = quantitySpinBox->value();
    return resultingProduct;
}

void ProductDialog::accept(){
    const Product enteredProduct = product();

    if (!originalProduct.has_value() && pendingBarcode.isEmpty()) {
        QMessageBox::warning(this, windowTitle(), "A barcode could not be created. Choose a category first.");
        return;
    }
    if (enteredProduct.name.isEmpty() || enteredProduct.name.contains(QChar('#'))) {
        QMessageBox::warning(this, windowTitle(), "Please enter a product name without the # character.");
        return;
    }
    if (enteredProduct.acronym.isEmpty() || enteredProduct.acronym.contains(QChar('#'))
        || enteredProduct.acronym.contains(QRegularExpression("\\s"))) {
        QMessageBox::warning(this, windowTitle(), "Please enter an acronym without spaces and without the # character.");
        return;
    }
    if (productCatalog.acronymInUse(enteredProduct.acronym, enteredProduct.productId)) {
        QMessageBox::warning(this, windowTitle(), "Another product already uses the acronym " + enteredProduct.acronym + ".");
        return;
    }
    if (enteredProduct.discountPerUnit > enteredProduct.unitPrice) {
        QMessageBox::warning(this, windowTitle(), "The discount per unit cannot be more than the unit price.");
        return;
    }

    const bool soldByCount = enteredProduct.measurementUnit == OrderConstants::measurementUnitCount;
    const double wholeQuantity = floor(enteredProduct.availableQuantity + 0.5);
    if (soldByCount && fabs(enteredProduct.availableQuantity - wholeQuantity) > quantityTolerance) {
        QMessageBox::warning(this, windowTitle(), "Products sold by count need a whole number as available quantity.");
        return;
    }
    QDialog::accept();
}