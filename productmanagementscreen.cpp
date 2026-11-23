#include "productmanagementscreen.h"
#include "barcodeUtils.h"
#include "filefieldformatter.h"
#include "orderconstants.h"
#include "productdialog.h"
#include <QAbstractItemView>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QImage>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

static const double quantityTolerance = 0.0000001;
static const double largestQuantity = 100000000.0;

ProductManagementScreen::ProductManagementScreen(ProductCatalog& products, CategoryCatalog& categories,
    const QString& dataFolderToUse, QWidget* parent)
    : QWidget(parent), productCatalog(products), categoryCatalog(categories), dataFolderPath(dataFolderToUse){
    productTable = new QTableWidget(0, 10);
    productTable->setHorizontalHeaderLabels({ "Product id", "Barcode", "Category", "Item", "Name", "Unit",
                                              "Unit price", "Tax", "Discount", "Available" });
    productTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    productTable->setSelectionMode(QAbstractItemView::SingleSelection);
    productTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    productTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QPushButton* addButton = new QPushButton("Add product...");
    QPushButton* editButton = new QPushButton("Edit selected...");
    QPushButton* inventoryButton = new QPushButton("Update inventory...");

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(addButton);
    buttonsLayout->addWidget(editButton);
    buttonsLayout->addWidget(inventoryButton);
    buttonsLayout->addStretch(1);

    messageLabel = new QLabel("Products and their available quantities (stored in Product.txt).");
    messageLabel->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(buttonsLayout);
    layout->addWidget(productTable, 1);
    layout->addWidget(messageLabel);

    connect(addButton, &QPushButton::clicked, this, [this] { addProduct(); });
    connect(editButton, &QPushButton::clicked, this, [this] { editSelectedProduct(); });
    connect(inventoryButton, &QPushButton::clicked, this, [this] { updateInventoryOfSelectedProduct(); });
    connect(productTable, &QTableWidget::cellDoubleClicked, this, [this] { editSelectedProduct(); });

    refresh();
}

void ProductManagementScreen::refresh(){
    const int previouslySelectedRow = productTable->currentRow();
    const vector<Product>& products = productCatalog.allProducts();
    productTable->setRowCount(static_cast<int>(products.size()));
    for (size_t productIndex = 0; productIndex < products.size(); ++productIndex) {
        const Product& product = products[productIndex];

        QString categoryText = product.categoryId;
        const optional<Category> category = categoryCatalog.findById(product.categoryId);
        if (category.has_value())
            categoryText = product.categoryId + " - " + category->name;

        QStringList cellTexts;
        cellTexts << product.productId << product.barcode << categoryText << product.acronym << product.name
            << product.measurementUnit << FileFieldFormatter::money(product.unitPrice)
            << FileFieldFormatter::money(product.taxPerUnit) << FileFieldFormatter::money(product.discountPerUnit)
            << FileFieldFormatter::quantity(product.availableQuantity);
        for (int columnIndex = 0; columnIndex < cellTexts.size(); ++columnIndex)
            productTable->setItem(static_cast<int>(productIndex), columnIndex, new QTableWidgetItem(cellTexts[columnIndex]));
    }
    if (previouslySelectedRow >= 0 && previouslySelectedRow < productTable->rowCount())
        productTable->setCurrentCell(previouslySelectedRow, 0);
}

optional<Product> ProductManagementScreen::selectedProduct() const{
    const int selectedRow = productTable->currentRow();
    const vector<Product>& products = productCatalog.allProducts();
    if (selectedRow < 0 || selectedRow >= static_cast<int>(products.size()))
        return nullopt;
    return products[selectedRow];
}

QString ProductManagementScreen::saveBarcodeImage(const Product& product) const{
    const QImage barcodeImage = makeBarcodeImage(product.barcode);
    if (barcodeImage.isNull())
        return "The barcode image could not be generated for " + product.barcode + ".";

    const QString savePath = QDir(dataFolderPath).absoluteFilePath(product.barcodeFilePath);
    QFileInfo(savePath).absoluteDir().mkpath(".");
    if (!barcodeImage.save(savePath))
        return "The barcode image could not be saved to " + savePath + ".";
    return QString();
}

void ProductManagementScreen::addProduct(){
    if (categoryCatalog.allCategories().empty()) {
        QMessageBox::information(this, "Add product", "Add at least one product category first.");
        return;
    }

    ProductDialog productDialog(productCatalog, categoryCatalog, optional<Product>(), this);
    if (productDialog.exec() != QDialog::Accepted)
        return;

    const Product newProduct = productDialog.product();
    QString problemDescription;
    if (!productCatalog.addProduct(newProduct, problemDescription)) {
        QMessageBox::critical(this, "Add product", problemDescription);
        return;
    }

    const QString imageProblem = saveBarcodeImage(newProduct);
    refresh();
    messageLabel->setText("Product " + newProduct.acronym + " was added with barcode " + newProduct.barcode + ".");
    if (!imageProblem.isEmpty())
        QMessageBox::warning(this, "Add product", "The product was added. " + imageProblem);
}

void ProductManagementScreen::editSelectedProduct(){
    const optional<Product> chosenProduct = selectedProduct();
    if (!chosenProduct.has_value()) {
        messageLabel->setText("Select a product in the table to edit it.");
        return;
    }

    ProductDialog productDialog(productCatalog, categoryCatalog, chosenProduct, this);
    if (productDialog.exec() != QDialog::Accepted)
        return;

    const Product changedProduct = productDialog.product();
    QString problemDescription;
    if (!productCatalog.updateProduct(changedProduct, problemDescription)) {
        QMessageBox::critical(this, "Edit product", problemDescription);
        return;
    }
    refresh();
    messageLabel->setText("Product " + changedProduct.acronym + " was updated.");
}

void ProductManagementScreen::updateInventoryOfSelectedProduct(){
    const optional<Product> chosenProduct = selectedProduct();
    if (!chosenProduct.has_value()) {
        messageLabel->setText("Select a product in the table to update its inventory.");
        return;
    }

    bool quantityWasEntered = false;
    const double newQuantity = QInputDialog::getDouble(
        this, "Update inventory",
        "Available quantity of " + chosenProduct->acronym + " - " + chosenProduct->name + ":",
        chosenProduct->availableQuantity, 0.0, largestQuantity, 3, &quantityWasEntered);
    if (!quantityWasEntered)
        return;

    const bool soldByCount = chosenProduct->measurementUnit == OrderConstants::measurementUnitCount;
    const double wholeQuantity = floor(newQuantity + 0.5);
    if (soldByCount && fabs(newQuantity - wholeQuantity) > quantityTolerance) {
        QMessageBox::warning(this, "Update inventory", "Products sold by count need a whole number.");
        return;
    }

    QString problemDescription;
    if (!productCatalog.setAvailableQuantity(chosenProduct->productId, newQuantity, problemDescription)) {
        QMessageBox::critical(this, "Update inventory", problemDescription);
        return;
    }
    refresh();
    messageLabel->setText("Inventory of " + chosenProduct->acronym + " is now "
        + FileFieldFormatter::quantity(newQuantity) + ".");
}