#include "heldordersdialog.h"

#include "filefieldformatter.h"

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

HeldOrdersDialog::HeldOrdersDialog(const vector<HeldOrderSummary>& ordersToShow, const QString& dialogTitle,
                                   const QString& chooseButtonText, QWidget* parent)
    : QDialog(parent), orders(ordersToShow)
{
    setWindowTitle(dialogTitle);
    resize(900, 380);

    ordersTable = new QTableWidget(static_cast<int>(orders.size()), 7);
    ordersTable->setHorizontalHeaderLabels({ "Order", "Date", "Time", "Customer", "Status", "Items", "Total (PKR)" });
    ordersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ordersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ordersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ordersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ordersTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);

    for (size_t rowIndex = 0; rowIndex < orders.size(); ++rowIndex) {
        const HeldOrderSummary& order = orders[rowIndex];
        QStringList cellTexts;
        cellTexts << order.orderId
                  << order.orderDate
                  << order.orderTime
                  << order.customerName
                  << order.status
                  << order.itemSummary
                  << FileFieldFormatter::money(order.totalPayable);
        for (int columnIndex = 0; columnIndex < cellTexts.size(); ++columnIndex)
            ordersTable->setItem(static_cast<int>(rowIndex), columnIndex, new QTableWidgetItem(cellTexts[columnIndex]));
    }
    ordersTable->selectRow(0);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText(chooseButtonText);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ordersTable, &QTableWidget::cellDoubleClicked, this, [this] { accept(); });

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Choose an order (double-click also works). Newest orders are first."));
    layout->addWidget(ordersTable, 1);
    layout->addWidget(buttonBox);
}

QString HeldOrdersDialog::selectedOrderId() const
{
    const int selectedRow = ordersTable->currentRow();
    if (selectedRow < 0 || selectedRow >= static_cast<int>(orders.size()))
        return QString();
    return orders[selectedRow].orderId;
}

bool HeldOrdersDialog::selectedOrderRequiresApproval() const
{
    const int selectedRow = ordersTable->currentRow();
    if (selectedRow < 0 || selectedRow >= static_cast<int>(orders.size()))
        return false;
    return orders[selectedRow].requiresApproval;
}
