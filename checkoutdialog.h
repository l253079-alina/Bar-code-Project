#pragma once

#include "billcalculator.h"
#include "checkoutservice.h"
#include "orderitem.h"
#include "standardnames.h"

#include <QDialog>

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;

class CheckoutDialog : public QDialog {
public:
    CheckoutDialog(const vector<OrderItem>& orderItemsToBill, double seasonalDiscountPercentToApply,
                   QWidget* parent = nullptr);
    void setCustomerDetails(const QString& customerName, const QString& customerContact);
    void setExistingPayment(const QString& paymentMode, const QString& merchantName);
    CheckoutDetails details() const;
    void accept() override;

private:
    vector<OrderItem> orderItems;
    double seasonalDiscountPercent = 0.0;
    double currentPayable = 0.0;
    double lastSuggestedAmount = 0.0;

    QLineEdit* customerNameLineEdit = nullptr;
    QLineEdit* customerContactLineEdit = nullptr;
    QComboBox* orderStatusComboBox = nullptr;
    QComboBox* paymentModeComboBox = nullptr;
    QLineEdit* merchantNameLineEdit = nullptr;
    QDoubleSpinBox* specialDiscountSpinBox = nullptr;
    QDoubleSpinBox* amountReceivedSpinBox = nullptr;
    QLabel* seasonalDiscountLabel = nullptr;
    QLabel* specialDiscountLabel = nullptr;
    QLabel* payableLabel = nullptr;
    QLabel* balanceLabel = nullptr;

    void updateMerchantFieldState();
    void updateAmounts();
    void updateBalanceLabel();
};
