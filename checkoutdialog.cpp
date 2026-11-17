#include "checkoutdialog.h"

#include "orderconstants.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

CheckoutDialog::CheckoutDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Generate bill");

    customerNameLineEdit = new QLineEdit("Walk-in");
    customerContactLineEdit = new QLineEdit;
    customerContactLineEdit->setPlaceholderText("Optional");

    orderStatusComboBox = new QComboBox;
    orderStatusComboBox->addItems({ OrderConstants::statusPaid, OrderConstants::statusInProgress,
                                    OrderConstants::statusOnHold, OrderConstants::statusArchived });

    paymentModeComboBox = new QComboBox;
    paymentModeComboBox->addItems({ OrderConstants::paymentModeCash, OrderConstants::paymentModeCard,
                                    OrderConstants::paymentModeWallet });

    merchantNameLineEdit = new QLineEdit;
    merchantNameLineEdit->setPlaceholderText("For example ABL or JazzCash");

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("Customer name", customerNameLineEdit);
    formLayout->addRow("Customer contact", customerContactLineEdit);
    formLayout->addRow("Order status", orderStatusComboBox);
    formLayout->addRow("Payment mode", paymentModeComboBox);
    formLayout->addRow("Payment merchant", merchantNameLineEdit);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(formLayout);
    layout->addWidget(buttonBox);

    connect(paymentModeComboBox, &QComboBox::currentTextChanged, this, [this] { updateMerchantFieldState(); });
    updateMerchantFieldState();
}

void CheckoutDialog::updateMerchantFieldState()
{
    const bool merchantIsNeeded = paymentModeComboBox->currentText() != OrderConstants::paymentModeCash;
    merchantNameLineEdit->setEnabled(merchantIsNeeded);
    if (!merchantIsNeeded)
        merchantNameLineEdit->clear();
}

CheckoutDetails CheckoutDialog::details() const
{
    CheckoutDetails checkoutDetails;
    checkoutDetails.customerName = customerNameLineEdit->text().trimmed();
    checkoutDetails.customerContact = customerContactLineEdit->text().trimmed();
    checkoutDetails.orderStatus = orderStatusComboBox->currentText();
    checkoutDetails.paymentMode = paymentModeComboBox->currentText();
    checkoutDetails.paymentMerchantName = merchantNameLineEdit->text().trimmed();
    return checkoutDetails;
}

void CheckoutDialog::accept()
{
    const CheckoutDetails enteredDetails = details();
    if (enteredDetails.customerName.isEmpty()) {
        QMessageBox::warning(this, "Generate bill", "Please enter the customer name.");
        return;
    }

    const bool merchantIsNeeded = enteredDetails.paymentMode != OrderConstants::paymentModeCash;
    if (merchantIsNeeded && enteredDetails.paymentMerchantName.isEmpty()) {
        QMessageBox::warning(this, "Generate bill", "Please enter the payment merchant name.");
        return;
    }
    QDialog::accept();
}
