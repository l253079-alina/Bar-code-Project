#pragma once

#include "billrepository.h"
#include "productcatalog.h"
#include "standardnames.h"

#include <QWidget>

class QLabel;
class QLineEdit;
class QTableWidget;

class BillsScreen : public QWidget {
public:
    BillsScreen(BillRepository& repository, ProductCatalog& catalog, QWidget* parent = nullptr);
    void refresh();

private:
    BillRepository& billRepository;
    ProductCatalog& productCatalog;
    vector<Bill> allBills;
    vector<size_t> shownBillPositions;

    QLineEdit* filterLineEdit = nullptr;
    QTableWidget* billTable = nullptr;
    QLabel* messageLabel = nullptr;

    void showFilteredBills();
    void openSelectedBill();
};
