#include "checkoutdialog.h"

#include "filefieldformatter.h"
#include "orderconstants.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

static const double largestAmount = 100000000.0;
static const double moneyTolerance = 0.005;

CheckoutDialog::CheckoutDialog(const vector<OrderItem>& orderItemsToBill, double seasonalDiscountPercentToApply,
                               QWidget* parent)
    : QDialog(parent), orderItems(orderItemsToBill), seasonalDiscountPercent(seasonalDiscountPercentToApply)
{
    setWindowTitle("Generate bill");

    customerNameLineEdit = new QLineEdit("Walk-in");
    customerContactLineEdit = new QLineEdit;
    customerContactLineEdit->setPlaceholderText("Optional");

    orderStatusComboBox = new QComboBox;
    orderStatusComboBox->addItems({ OrderConstants::statusPaid, OrderConstants::statusInProgress,
                                    OrderConstants::statusArchived });

    paymentModeComboBox = new QComboBox;
    paymentModeComboBox->addItems({ OrderConstants::paymentModeCash, OrderConstants::paymentModeCard,
                                    OrderConstants::paymentModeWallet });

    merchantNameLineEdit = new QLineEdit;
    merchantNameLineEdit->setPlaceholderText("For example ABL or JazzCash");

    specialDiscountSpinBox = new QDoubleSpinBox;
    specialDiscountSpinBox->setDecimals(2);
    specialDiscountSpinBox->setRange(0.0, OrderConstants::maximumSpecialDiscountPercent);
    specialDiscountSpinBox->setSuffix(" %");

    amountReceivedSpinBox = new QDoubleSpinBox;
    amountReceivedSpinBox->setDecimals(2);
    amountReceivedSpinBox->setRange(0.0, largestAmount);

    seasonalDiscountLabel = new QLabel;
    specialDiscountLabel = new QLabel;
    payableLabel = new QLabel;
    payableLabel->setStyleSheet("font-weight: bold;");
    balanceLabel = new QLabel;

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("Customer name", customerNameLineEdit);
    formLayout->addRow("Customer contact", customerContactLineEdit);
    formLayout->addRow("Order status", orderStatusComboBox);
    formLayout->addRow("Payment mode", paymentModeComboBox);
    formLayout->addRow("Payment merchant", merchantNameLineEdit);
    formLayout->addRow("Special discount (max 8 %)", specialDiscountSpinBox);
    formLayout->addRow("Seasonal discount", seasonalDiscountLabel);
    formLayout->addRow("Special discount amount", specialDiscountLabel);
    formLayout->addRow("Total payable", payableLabel);
    formLayout->addRow("Amount received", amountReceivedSpinBox);
    formLayout->addRow("Balance", balanceLabel);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(formLayout);
    layout->addWidget(buttonBox);

    connect(paymentModeComboBox, &QComboBox::currentTextChanged, this, [this] {
        updateMerchantFieldState();
        updateAmounts();
    });
    connect(orderStatusComboBox, &QComboBox::currentTextChanged, this, [this] { updateAmounts(); });
    connect(specialDiscountSpinBox, &QDoubleSpinBox::valueChanged, this, [this] { updateAmounts(); });
    connect(amountReceivedSpinBox, &QDoubleSpinBox::valueChanged, this, [this] { updateBalanceLabel(); });

    updateMerchantFieldState();
    updateAmounts();
}

void CheckoutDialog::setCustomerDetails(const QString& customerName, const QString& customerContact)
{
    if (!customerName.trimmed().isEmpty())
        customerNameLineEdit->setText(customerName.trimmed());
    customerContactLineEdit->setText(customerContact.trimmed());
}

void CheckoutDialog::setExistingPayment(const QString& paymentMode, const QString& merchantName)
{
    const int paymentModeIndex = paymentModeComboBox->findText(paymentMode);
    if (paymentModeIndex >= 0)
        paymentModeComboBox->setCurrentIndex(paymentModeIndex);
    if (merchantNameLineEdit->isEnabled())
        merchantNameLineEdit->setText(merchantName);
}

void CheckoutDialog::updateMerchantFieldState()
{
    const bool merchantIsNeeded = paymentModeComboBox->currentText() != OrderConstants::paymentModeCash;
    merchantNameLineEdit->setEnabled(merchantIsNeeded);
    if (!merchantIsNeeded)
        merchantNameLineEdit->clear();
}

void CheckoutDialog::updateAmounts()
{
    const BillTotals totals =
        BillCalculator::calculateTotals(orderItems, seasonalDiscountPercent, specialDiscountSpinBox->value());
    currentPayable = totals.payable;

    seasonalDiscountLabel->setText(FileFieldFormatter::money(totals.seasonalDiscount) + " ("
                                   + FileFieldFormatter::quantity(seasonalDiscountPercent) + " %)");
    specialDiscountLabel->setText(FileFieldFormatter::money(totals.specialDiscount));
    payableLabel->setText("PKR " + FileFieldFormatter::money(totals.payable));

    const bool orderIsPaid = orderStatusComboBox->currentText() == OrderConstants::statusPaid;
    const bool paidInCash = orderIsPaid && paymentModeComboBox->currentText() == OrderConstants::paymentModeCash;

    if (!orderIsPaid) {
        amountReceivedSpinBox->setEnabled(false);
        amountReceivedSpinBox->setValue(0.0);
        lastSuggestedAmount = 0.0;
        updateBalanceLabel();
        return;
    }

    const bool amountIsStillSuggested = fabs(amountReceivedSpinBox->value() - lastSuggestedAmount) < moneyTolerance;
    if (amountIsStillSuggested || !paidInCash)
        amountReceivedSpinBox->setValue(totals.payable);
    lastSuggestedAmount = totals.payable;
    amountReceivedSpinBox->setEnabled(paidInCash);
    updateBalanceLabel();
}

void CheckoutDialog::updateBalanceLabel()
{
    const bool orderIsPaid = orderStatusComboBox->currentText() == OrderConstants::statusPaid;
    if (!orderIsPaid) {
        balanceLabel->setText("Nothing is received for an order that is not paid");
        return;
    }

    const double difference = amountReceivedSpinBox->value() - currentPayable;
    if (difference >= -moneyTolerance)
        balanceLabel->setText("Return PKR " + FileFieldFormatter::money(max(difference, 0.0)));
    else
        balanceLabel->setText("Short by PKR " + FileFieldFormatter::money(-difference));
}

CheckoutDetails CheckoutDialog::details() const
{
    CheckoutDetails checkoutDetails;
    checkoutDetails.customerName = customerNameLineEdit->text().trimmed();
    checkoutDetails.customerContact = customerContactLineEdit->text().trimmed();
    checkoutDetails.orderStatus = orderStatusComboBox->currentText();
    checkoutDetails.paymentMode = paymentModeComboBox->currentText();
    checkoutDetails.paymentMerchantName = merchantNameLineEdit->text().trimmed();
    checkoutDetails.specialDiscountPercent = specialDiscountSpinBox->value();
    if (checkoutDetails.orderStatus == OrderConstants::statusPaid)
        checkoutDetails.amountReceived = amountReceivedSpinBox->value();
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

    const bool orderIsPaid = enteredDetails.orderStatus == OrderConstants::statusPaid;
    if (orderIsPaid && enteredDetails.amountReceived + moneyTolerance < currentPayable) {
        QMessageBox::warning(this, "Generate bill", "The amount received is less than the total payable.");
        return;
    }
    QDialog::accept();
}
