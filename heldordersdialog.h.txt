#pragma once

#include "heldorderservice.h"
#include "standardnames.h"

#include <QDialog>

class QTableWidget;

class HeldOrdersDialog : public QDialog {
public:
    HeldOrdersDialog(const vector<HeldOrderSummary>& ordersToShow, const QString& dialogTitle,
                     const QString& chooseButtonText, QWidget* parent = nullptr);
    QString selectedOrderId() const;
    bool selectedOrderRequiresApproval() const;

private:
    vector<HeldOrderSummary> orders;
    QTableWidget* ordersTable = nullptr;
};
