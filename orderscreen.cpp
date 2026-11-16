#include "orderscreen.h"

#include "billcalculator.h"
#include "billdialog.h"
#include "checkoutdialog.h"
#include "filefieldformatter.h"
#include "productdescriptionformatter.h"
#include "scanpanel.h"

#include <QAbstractItemView>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

OrderScreen::OrderScreen(ProductCatalog& catalog, CheckoutService& checkout, QWidget* parent): QWidget(parent), productCatalog(catalog), checkoutService(checkout){
    scanPanel = new ScanPanel;

    quantitySpinBox = new QDoubleSpinBox;
    quantitySpinBox->setDecimals(3);
    quantitySpinBox->setRange(0.001, 100000.0);
    quantitySpinBox->setValue(1.0);

    QHBoxLayout* quantityLayout = new QHBoxLayout;
    quantityLayout->addWidget(new QLabel("Quantity for the next scanned product:"));
    quantityLayout->addWidget(quantitySpinBox);
    quantityLayout->addStretch(1);

    orderTable = new QTableWidget(0, 8);
    orderTable->setHorizontalHeaderLabels({ "Item", "Name", "Unit", "Quantity", "Unit price", "Discount", "Tax", "Line total" });
    orderTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    orderTable->setSelectionMode(QAbstractItemView::SingleSelection);
    orderTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    orderTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QPushButton* descriptionButton = new QPushButton("Description");
    QPushButton* removeButton = new QPushButton("Remove selected");
    QPushButton* clearButton = new QPushButton("Clear order");
    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(descriptionButton);
    buttonsLayout->addWidget(removeButton);
    buttonsLayout->addWidget(clearButton);

    totalsLabel = new QLabel;
    messageLabel = new QLabel("Scan a product to add it to the order.");
    messageLabel->setWordWrap(true);
    QPushButton* generateBillButton = new QPushButton("Generate bill...");

    QVBoxLayout* orderLayout = new QVBoxLayout;
    orderLayout->addLayout(quantityLayout);
    orderLayout->addWidget(orderTable, 1);
    orderLayout->addLayout(buttonsLayout);
    orderLayout->addWidget(totalsLabel);
    orderLayout->addWidget(messageLabel);
    orderLayout->addWidget(generateBillButton);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(scanPanel, 2);
    layout->addLayout(orderLayout, 3);

    connect(scanPanel, &ScanPanel::barcodeScanned, this,
            [this](const QString& scannedBarcode) { handleScannedBarcode(scannedBarcode); });
    connect(descriptionButton, &QPushButton::clicked, this, [this] { showDescription(); });
    connect(removeButton, &QPushButton::clicked, this, [this] { removeSelectedItem(); });
    connect(clearButton, &QPushButton::clicked, this, [this] { clearOrder(); });
    connect(generateBillButton, &QPushButton::clicked, this, [this] { generateBill(); });

    refreshTotals();
}

void OrderScreen::stopScanning(){
    scanPanel->stopScanning();
}

void OrderScreen::showMessage(const QString& messageText){
    messageLabel->setText(messageText);
}

void OrderScreen::handleScannedBarcode(const QString& scannedBarcode){
    const optional<Product> foundProduct = productCatalog.findByBarcode(scannedBarcode);
    if (!foundProduct.has_value()) {
        showMessage("No product found for barcode " + scannedBarcode);
        return;
    }

    lastScannedProduct = foundProduct;
    const double requiredQuantity = quantitySpinBox->value();
    QString rejectionReason;
    if (!shoppingCart.addProduct(*foundProduct, requiredQuantity, rejectionReason)) {
        showMessage(rejectionReason);
        return;
    }

    refreshOrderTable();
    refreshTotals();
    showMessage(QString("Added %1 x %2 (%3)")
                    .arg(FileFieldFormatter::quantity(requiredQuantity), foundProduct->acronym, foundProduct->name));
}

void OrderScreen::refreshOrderTable(){
    const vector<OrderItem>& orderItems = shoppingCart.items();
    orderTable->setRowCount(static_cast<int>(orderItems.size()));
    for (size_t itemIndex = 0; itemIndex < orderItems.size(); ++itemIndex) {
        const OrderItem& item = orderItems[itemIndex];
        const double unitPriceToPay = item.product.unitPrice - item.product.discountPerUnit + item.product.taxPerUnit;

        QStringList cellTexts;
        cellTexts << item.product.acronym
                  << item.product.name
                  << item.product.measurementUnit
                  << FileFieldFormatter::quantity(item.quantity)
                  << FileFieldFormatter::money(item.product.unitPrice)
                  << FileFieldFormatter::money(item.product.discountPerUnit)
                  << FileFieldFormatter::money(item.product.taxPerUnit)
                  << FileFieldFormatter::money(unitPriceToPay * item.quantity);
        for (int columnIndex = 0; columnIndex < cellTexts.size(); ++columnIndex)
            orderTable->setItem(static_cast<int>(itemIndex), columnIndex, new QTableWidgetItem(cellTexts[columnIndex]));
    }
}

void OrderScreen::refreshTotals(){
    const BillTotals totals = BillCalculator::calculateTotals(shoppingCart.items());
    totalsLabel->setText(QString("Without tax: %1    Discount: %2    Tax: %3    Payable: PKR %4")
                             .arg(FileFieldFormatter::money(totals.withoutTax),
                                  FileFieldFormatter::money(totals.discount),
                                  FileFieldFormatter::money(totals.tax),
                                  FileFieldFormatter::money(totals.payable)));
}

void OrderScreen::showDescription(){
    const int selectedRow = orderTable->currentRow();
    const int itemCount = static_cast<int>(shoppingCart.items().size());
    if (selectedRow >= 0 && selectedRow < itemCount) {
        QMessageBox::information(this, "Product description",
                                 ProductDescriptionFormatter::describe(shoppingCart.items()[selectedRow].product));
        return;
    }
    if (lastScannedProduct.has_value()) {
        QMessageBox::information(this, "Product description",
                                 ProductDescriptionFormatter::describe(*lastScannedProduct));
        return;
    }
    QMessageBox::information(this, "Product description", "Select a product in the order or scan one first.");
}

void OrderScreen::removeSelectedItem(){
    const int selectedRow = orderTable->currentRow();
    if (selectedRow < 0) {
        showMessage("Select a product in the order to remove it.");
        return;
    }
    shoppingCart.removeItemAt(selectedRow);
    refreshOrderTable();
    refreshTotals();
    showMessage("Product removed from the order.");
}

void OrderScreen::clearOrder(){
    shoppingCart.clear();
    lastScannedProduct.reset();
    refreshOrderTable();
    refreshTotals();
    showMessage("The order was cleared.");
}

void OrderScreen::generateBill(){
    if (shoppingCart.isEmpty()) {
        QMessageBox::warning(this, "Generate bill", "Add at least one product to the order first.");
        return;
    }

    CheckoutDialog checkoutDialog(this);
    if (checkoutDialog.exec() != QDialog::Accepted)
        return;

    Bill createdBill;
    QString problemDescription;
    if (!checkoutService.completeCheckout(shoppingCart.items(), checkoutDialog.details(), createdBill, problemDescription)) {
        QMessageBox::critical(this, "Generate bill", problemDescription);
        return;
    }

    BillDialog billDialog(createdBill, this);
    billDialog.exec();

    shoppingCart.clear();
    lastScannedProduct.reset();
    refreshOrderTable();
    refreshTotals();
    showMessage("Bill " + createdBill.billId + " was saved.");
}
