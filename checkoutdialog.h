#pragma once

#include "checkoutservice.h"

#include <QDialog>

class QComboBox;
class QLineEdit;

class CheckoutDialog : public QDialog {
public:
    explicit CheckoutDialog(QWidget* parent = nullptr);
    CheckoutDetails details() const;
    void accept() override;

private:
    QLineEdit* customerNameLineEdit = nullptr;
    QLineEdit* customerContactLineEdit = nullptr;
    QComboBox* orderStatusComboBox = nullptr;
    QComboBox* paymentModeComboBox = nullptr;
    QLineEdit* merchantNameLineEdit = nullptr;

    void updateMerchantFieldState();
};
