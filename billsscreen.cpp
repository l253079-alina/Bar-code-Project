#include "billsscreen.h"

#include "billdialog.h"
#include "filefieldformatter.h"

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

BillsScreen::BillsScreen(BillRepository& repository, ProductCatalog& catalog, QWidget* parent)
    : QWidget(parent), billRepository(repository), productCatalog(catalog)
{
    filterLineEdit = new QLineEdit;
    filterLineEdit->setPlaceholderText("Filter by bill id, customer, order id, date or status");
    QPushButton* refreshButton = new QPushButton("Refresh");
    QPushButton* openButton = new QPushButton("View / save as PDF...");

    billTable = new QTableWidget(0, 7);
    billTable->setHorizontalHeaderLabels({ "Bill", "Date", "Time", "Customer", "Order", "Status", "Payable (PKR)" });
    billTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    billTable->setSelectionMode(QAbstractItemView::SingleSelection);
    billTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    billTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    messageLabel = new QLabel("Select a bill to view it or to save it as a PDF.");
    messageLabel->setWordWrap(true);

    QHBoxLayout* topLayout = new QHBoxLayout;
    topLayout->addWidget(filterLineEdit, 1);
    topLayout->addWidget(refreshButton);
    topLayout->addWidget(openButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(topLayout);
    layout->addWidget(billTable, 1);
    layout->addWidget(messageLabel);

    connect(filterLineEdit, &QLineEdit::textChanged, this, [this] { showFilteredBills(); });
    connect(refreshButton, &QPushButton::clicked, this, [this] { refresh(); });
    connect(openButton, &QPushButton::clicked, this, [this] { openSelectedBill(); });
    connect(billTable, &QTableWidget::cellDoubleClicked, this, [this] { openSelectedBill(); });

    refresh();
}

void BillsScreen::refresh()
{
    QString problemDescription;
    if (!billRepository.loadBills(allBills, problemDescription)) {
        QMessageBox::critical(this, "Bills", problemDescription);
        return;
    }
    showFilteredBills();
}

void BillsScreen::showFilteredBills()
{
    const QString filterText = filterLineEdit->text().trimmed();
    shownBillPositions.clear();
    for (size_t position = allBills.size(); position > 0; --position) {
        const Bill& bill = allBills[position - 1];
        const QString searchableText = bill.billId + " " + bill.customerName + " " + bill.orderId + " "
            + bill.billDate + " " + bill.paymentStatus;
        if (filterText.isEmpty() || searchableText.contains(filterText, Qt::CaseInsensitive))
            shownBillPositions.push_back(position - 1);
    }

    billTable->setRowCount(static_cast<int>(shownBillPositions.size()));
    for (size_t rowIndex = 0; rowIndex < shownBillPositions.size(); ++rowIndex) {
        const Bill& bill = allBills[shownBillPositions[rowIndex]];
        QStringList cellTexts;
        cellTexts << bill.billId << bill.billDate << bill.billTime << bill.customerName << bill.orderId
                  << bill.paymentStatus << FileFieldFormatter::money(bill.totalPayable);
        for (int columnIndex = 0; columnIndex < cellTexts.size(); ++columnIndex)
            billTable->setItem(static_cast<int>(rowIndex), columnIndex, new QTableWidgetItem(cellTexts[columnIndex]));
    }
}

void BillsScreen::openSelectedBill()
{
    const int selectedRow = billTable->currentRow();
    if (selectedRow < 0 || selectedRow >= static_cast<int>(shownBillPositions.size())) {
        messageLabel->setText("Select a bill in the table first.");
        return;
    }

    Bill selectedBill = allBills[shownBillPositions[selectedRow]];
    for (BillLine& line : selectedBill.lines) {
        const optional<Product> product = productCatalog.findByAcronym(line.productAcronym);
        if (!product.has_value())
            continue;
        line.productName = product->name;
        line.taxPerUnit = product->taxPerUnit;
    }

    BillDialog billDialog(selectedBill, this);
    billDialog.exec();
}
