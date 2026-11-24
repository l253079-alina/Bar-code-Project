#include "orderscreen.h"

#include "billcalculator.h"
#include "billdialog.h"
#include "checkoutdialog.h"
#include "filefieldformatter.h"
#include "heldordersdialog.h"
#include "orderconstants.h"
#include "productdescriptionformatter.h"
#include "scanpanel.h"

#include <QAbstractItemView>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

static const int tableColumnCount = 9;
static const int changeColumnIndex = 8;

OrderScreen::OrderScreen(ProductCatalog& catalog, CheckoutService& checkout, HeldOrderService& heldOrders,ManagerApprovalService& approvalService, QWidget* parent)
    : QWidget(parent),
      productCatalog(catalog),
      checkoutService(checkout),
      heldOrderService(heldOrders),
      managerApprovalService(approvalService)
{
    scanPanel = new ScanPanel;

    orderTitleLabel = new QLabel;
    orderTitleLabel->setStyleSheet("font-weight: bold;");

    customerNameLineEdit = new QLineEdit;
    customerNameLineEdit->setPlaceholderText("Optional");

    quantitySpinBox = new QDoubleSpinBox;
    quantitySpinBox->setDecimals(3);
    quantitySpinBox->setRange(0.001, 100000.0);
    quantitySpinBox->setValue(1.0);

    QHBoxLayout* customerLayout = new QHBoxLayout;
    customerLayout->addWidget(new QLabel("Customer:"));
    customerLayout->addWidget(customerNameLineEdit, 1);
    customerLayout->addSpacing(12);
    customerLayout->addWidget(new QLabel("Quantity for the next scan:"));
    customerLayout->addWidget(quantitySpinBox);

    orderTable = new QTableWidget(0, tableColumnCount);
    orderTable->setHorizontalHeaderLabels({ "Item", "Name", "Unit", "Quantity", "Unit price", "Discount", "Tax", "Line total", "Change" });
    orderTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    orderTable->setSelectionMode(QAbstractItemView::SingleSelection);
    orderTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    orderTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    orderTable->horizontalHeader()->setSectionResizeMode(changeColumnIndex, QHeaderView::Fixed);
    orderTable->setColumnWidth(changeColumnIndex, 84);
    orderTable->verticalHeader()->setDefaultSectionSize(34);

    QPushButton* descriptionButton = new QPushButton("Description");
    QPushButton* removeButton = new QPushButton("Remove selected");
    QPushButton* clearButton = new QPushButton("Clear order");
    QHBoxLayout* editButtonsLayout = new QHBoxLayout;
    editButtonsLayout->addWidget(descriptionButton);
    editButtonsLayout->addWidget(removeButton);
    editButtonsLayout->addWidget(clearButton);

    QPushButton* holdButton = new QPushButton("Put on hold");
    QPushButton* resumeButton = new QPushButton("Resume order...");
    QPushButton* editOrderButton = new QPushButton("Edit existing order...");
    QHBoxLayout* holdButtonsLayout = new QHBoxLayout;
    holdButtonsLayout->addWidget(holdButton);
    holdButtonsLayout->addWidget(resumeButton);
    holdButtonsLayout->addWidget(editOrderButton);

    totalsLabel = new QLabel;
    messageLabel = new QLabel("Scan a product to add it to the order.");
    messageLabel->setWordWrap(true);
    QPushButton* generateBillButton = new QPushButton("Generate bill...");

    QVBoxLayout* orderLayout = new QVBoxLayout;
    orderLayout->addWidget(orderTitleLabel);
    orderLayout->addLayout(customerLayout);
    orderLayout->addWidget(orderTable, 1);
    orderLayout->addLayout(editButtonsLayout);
    orderLayout->addLayout(holdButtonsLayout);
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
    connect(holdButton, &QPushButton::clicked, this, [this] { holdOrder(); });
    connect(resumeButton, &QPushButton::clicked, this, [this] { resumeOrder(); });
    connect(editOrderButton, &QPushButton::clicked, this, [this] { editExistingOrder(); });
    connect(generateBillButton, &QPushButton::clicked, this, [this] { generateBill(); });

    refreshOrderTitle();
    refreshTotals();
}

void OrderScreen::stopScanning(){
    scanPanel->stopScanning();
}

void OrderScreen::showMessage(const QString& messageText){
    messageLabel->setText(messageText);
}

void OrderScreen::refreshLater(){
    QTimer::singleShot(0, this, [this] {
        refreshOrderTable();
        refreshTotals();
    });
}

void OrderScreen::startNewOrder(){
    shoppingCart.clear();
    lastScannedProduct.reset();
    existingOrder.reset();
    customerNameLineEdit->clear();
    refreshOrderTable();
    refreshTotals();
    refreshOrderTitle();
}

bool OrderScreen::orderIsSavedAsBilledOrder() const{
    return existingOrder.has_value() && existingOrder->status != OrderConstants::statusOnHold;
}

void OrderScreen::refreshOrderTitle(){
    if (!existingOrder.has_value()) {
        orderTitleLabel->setText("New order");
        return;
    }

    if (existingOrder->status == OrderConstants::statusOnHold)
        orderTitleLabel->setText("Order " + existingOrder->orderId + " (resumed from on-hold, started "+ existingOrder->orderDate + " " + existingOrder->orderTime + ")");
    else
        orderTitleLabel->setText("Order " + existingOrder->orderId + " (editing a " + existingOrder->status+ " order, started " + existingOrder->orderDate + " " + existingOrder->orderTime + ")");
}

QWidget* OrderScreen::createQuantityButtons(int rowIndex){
    QPushButton* minusButton = new QPushButton("-");
    QPushButton* plusButton = new QPushButton("+");
    minusButton->setFixedWidth(32);
    plusButton->setFixedWidth(32);
    minusButton->setToolTip("Decrease the quantity");
    plusButton->setToolTip("Increase the quantity");

    QWidget* buttonsWidget = new QWidget;
    QHBoxLayout* buttonsLayout = new QHBoxLayout(buttonsWidget);
    buttonsLayout->setContentsMargins(2, 1, 2, 1);
    buttonsLayout->setSpacing(4);
    buttonsLayout->addWidget(minusButton);
    buttonsLayout->addWidget(plusButton);

    connect(minusButton, &QPushButton::clicked, this, [this, rowIndex] { changeQuantityOfRow(rowIndex, -1); });
    connect(plusButton, &QPushButton::clicked, this, [this, rowIndex] { changeQuantityOfRow(rowIndex, 1); });
    return buttonsWidget;
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

    quantitySpinBox->setValue(1.0);
    refreshOrderTable();
    refreshTotals();
    showMessage(QString("Added %1 x %2 (%3)")
                    .arg(FileFieldFormatter::quantity(requiredQuantity), foundProduct->acronym, foundProduct->name));
}

void OrderScreen::changeQuantityOfRow(int rowIndex, int direction){
    const vector<OrderItem>& orderItems = shoppingCart.items();
    if (rowIndex < 0 || rowIndex >= static_cast<int>(orderItems.size()))
        return;

    const Product product = orderItems[rowIndex].product;
    const size_t itemCountBefore = orderItems.size();
    const double quantityChange = ShoppingCart::quantityStepFor(product) * direction;

    QString rejectionReason;
    if (!shoppingCart.changeQuantityAt(rowIndex, quantityChange, rejectionReason)) {
        showMessage(rejectionReason);
        return;
    }

    if (shoppingCart.items().size() < itemCountBefore) {
        showMessage("Removed " + product.acronym + " from the order.");
    } else {
        const double newQuantity = shoppingCart.items()[rowIndex].quantity;
        showMessage(product.acronym + " quantity is now " + FileFieldFormatter::quantity(newQuantity) + ".");
    }
    refreshLater();
}

void OrderScreen::refreshOrderTable(){
    const int previouslySelectedRow = orderTable->currentRow();
    const vector<OrderItem>& orderItems = shoppingCart.items();
    orderTable->setRowCount(static_cast<int>(orderItems.size()));
    for (size_t itemIndex = 0; itemIndex < orderItems.size(); ++itemIndex) {
        const OrderItem& item = orderItems[itemIndex];
        const int rowIndex = static_cast<int>(itemIndex);
        const double unitPriceToPay = item.product.unitPrice - item.product.discountPerUnit + item.product.taxPerUnit;

        QStringList cellTexts;
        cellTexts << item.product.acronym<< item.product.name<< item.product.measurementUnit<< FileFieldFormatter::quantity(item.quantity)<< FileFieldFormatter::money(item.product.unitPrice)<< FileFieldFormatter::money(item.product.discountPerUnit)<< FileFieldFormatter::money(item.product.taxPerUnit)<< FileFieldFormatter::money(unitPriceToPay * item.quantity);
        for (int columnIndex = 0; columnIndex < cellTexts.size(); ++columnIndex)
            orderTable->setItem(rowIndex, columnIndex, new QTableWidgetItem(cellTexts[columnIndex]));

        orderTable->setCellWidget(rowIndex, changeColumnIndex, createQuantityButtons(rowIndex));
    }

    if (previouslySelectedRow >= 0 && previouslySelectedRow < orderTable->rowCount())
        orderTable->setCurrentCell(previouslySelectedRow, 0);
}

void OrderScreen::refreshTotals(){
    const BillTotals totals = BillCalculator::calculateTotals(shoppingCart.items());
    totalsLabel->setText(QString("Without tax: %1    Discount: %2    Tax: %3    Payable: PKR %4").arg(FileFieldFormatter::money(totals.withoutTax),FileFieldFormatter::money(totals.discount),FileFieldFormatter::money(totals.tax),FileFieldFormatter::money(totals.payable)));
}

void OrderScreen::showDescription(){
    const int selectedRow = orderTable->currentRow();
    const int itemCount = static_cast<int>(shoppingCart.items().size());
    if (selectedRow >= 0 && selectedRow < itemCount) {
        QMessageBox::information(this, "Product description",ProductDescriptionFormatter::describe(shoppingCart.items()[selectedRow].product));
        return;
    }
    if (lastScannedProduct.has_value()) {
        QMessageBox::information(this, "Product description",ProductDescriptionFormatter::describe(*lastScannedProduct));
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
    const bool orderWasSavedBefore = existingOrder.has_value();
    startNewOrder();
    if (orderWasSavedBefore)
        showMessage("The screen was cleared. The saved order was not changed.");
    else
        showMessage("The order was cleared.");
}

void OrderScreen::holdOrder(){
    if (shoppingCart.isEmpty()) {
        showMessage("Add at least one product before putting the order on hold.");
        return;
    }
    if (orderIsSavedAsBilledOrder()) {
        showMessage("An order that is being edited cannot be put on hold. Generate the bill or clear the screen.");
        return;
    }

    QString heldOrderId;
    QString problemDescription;
    if (!heldOrderService.holdOrder(shoppingCart.items(), customerNameLineEdit->text(), existingOrder,heldOrderId, problemDescription)) {
        QMessageBox::critical(this, "Put on hold", problemDescription);
        return;
    }

    startNewOrder();
    showMessage("Order " + heldOrderId + " was put on hold. Press Resume order to continue it.");
}

void OrderScreen::resumeOrder(){
    vector<HeldOrderSummary> heldOrders;
    QString problemDescription;
    if (!heldOrderService.findHeldOrders(heldOrders, problemDescription)) {
        QMessageBox::critical(this, "Resume order", problemDescription);
        return;
    }
    if (heldOrders.empty()) {
        QMessageBox::information(this, "Resume order", "There are no orders on hold.");
        return;
    }

    if (heldOrders.size() == 1) {
        openOrder(heldOrders.front().orderId);
        return;
    }

    HeldOrdersDialog heldOrdersDialog(heldOrders, "Orders on hold", "Resume", this);
    if (heldOrdersDialog.exec() != QDialog::Accepted)
        return;
    openOrder(heldOrdersDialog.selectedOrderId());
}

void OrderScreen::editExistingOrder(){
    vector<HeldOrderSummary> editableOrders;
    QString problemDescription;
    if (!heldOrderService.findEditableOrders(editableOrders, problemDescription)) {
        QMessageBox::critical(this, "Edit existing order", problemDescription);
        return;
    }
    if (editableOrders.empty()) {
        QMessageBox::information(this, "Edit existing order","There are no orders that can be edited. In-progress orders can be edited at any time. ""Paid orders can be edited for "+ QString::number(OrderConstants::paidOrderEditWindowDays)+ " days after payment, with a manager's approval.");
        return;
    }

    HeldOrdersDialog editableOrdersDialog(editableOrders, "Edit existing order", "Edit", this);
    if (editableOrdersDialog.exec() != QDialog::Accepted)
        return;

    const QString chosenOrderId = editableOrdersDialog.selectedOrderId();
    if (chosenOrderId.isEmpty())
        return;
    if (existingOrder.has_value() && existingOrder->orderId == chosenOrderId) {
        showMessage("Order " + chosenOrderId + " is already open.");
        return;
    }

    if (editableOrdersDialog.selectedOrderRequiresApproval()) {
        if (!managerApprovalService.askForApproval(this)) {
            showMessage("Order " + chosenOrderId + " was not opened because the manager did not approve the change.");
            return;
        }
    }
    openOrder(chosenOrderId);
}

void OrderScreen::openOrder(const QString& orderId){
    if (existingOrder.has_value() && existingOrder->orderId == orderId) {
        showMessage("Order " + orderId + " is already open.");
        return;
    }

    HeldOrderContents openedOrderContents;
    QString problemDescription;
    if (!heldOrderService.openOrder(orderId, openedOrderContents, problemDescription)) {
        QMessageBox::critical(this, "Open order", problemDescription);
        return;
    }

    QString putOnHoldText;
    if (!shoppingCart.isEmpty()) {
        if (orderIsSavedAsBilledOrder()) {
            const QMessageBox::StandardButton answer = QMessageBox::question(this, "Open order","The order " + existingOrder->orderId + " has changes that are not saved. Discard them?");
            if (answer != QMessageBox::Yes)
                return;
        } else {
            QString currentOrderId;
            if (!heldOrderService.holdOrder(shoppingCart.items(), customerNameLineEdit->text(), existingOrder,currentOrderId, problemDescription)) {
                QMessageBox::critical(this, "Open order", problemDescription);
                return;
            }
            putOnHoldText = " The previous order was put on hold as " + currentOrderId + ".";
        }
    }

    startNewOrder();
    shoppingCart.setReservedItems(openedOrderContents.orderInfo.reservedItems);
    QStringList notRestoredReasons;
    for (const OrderItem& item : openedOrderContents.items) {
        QString rejectionReason;
        if (!shoppingCart.addProduct(item.product, item.quantity, rejectionReason))
            notRestoredReasons << rejectionReason;
    }
    existingOrder = openedOrderContents.orderInfo;
    customerNameLineEdit->setText(openedOrderContents.orderInfo.customerName);
    refreshOrderTable();
    refreshTotals();
    refreshOrderTitle();

    if (openedOrderContents.orderInfo.status == OrderConstants::statusOnHold)
        showMessage("Order " + orderId + " was resumed." + putOnHoldText);
    else
        showMessage("Order " + orderId + " is open for editing. Generate the bill to save the changes." + putOnHoldText);

    QStringList warnings;
    if (!openedOrderContents.warning.isEmpty())
        warnings << openedOrderContents.warning;
    warnings << notRestoredReasons;
    if (!warnings.isEmpty())
        QMessageBox::warning(this, "Open order", warnings.join("\n"));
}

void OrderScreen::generateBill(){
    if (shoppingCart.isEmpty()) {
        QMessageBox::warning(this, "Generate bill", "Add at least one product to the order first.");
        return;
    }

    QString customerContact;
    if (existingOrder.has_value())
        customerContact = existingOrder->customerContact;

    const double seasonalPercent = checkoutService.previewSeasonalPercent(existingOrder);
    CheckoutDialog checkoutDialog(shoppingCart.items(), seasonalPercent, this);
    checkoutDialog.setCustomerDetails(customerNameLineEdit->text(), customerContact);
    if (existingOrder.has_value())
        checkoutDialog.setExistingPayment(existingOrder->paymentMode, existingOrder->paymentMerchantName);
    if (checkoutDialog.exec() != QDialog::Accepted)
        return;

    Bill createdBill;
    QString problemDescription;
    if (!checkoutService.completeCheckout(shoppingCart.items(), checkoutDialog.details(), existingOrder,createdBill, problemDescription)) {
        QMessageBox::critical(this, "Generate bill", problemDescription);
        return;
    }

    BillDialog billDialog(createdBill, this);
    billDialog.exec();

    startNewOrder();
    showMessage("Bill " + createdBill.billId + " was saved.");
}
