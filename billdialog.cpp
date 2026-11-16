#include "billdialog.h"
#include "billformatter.h"
#include <QFontDatabase>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

BillDialog::BillDialog(const Bill& bill, QWidget* parent) : QDialog(parent){
    setWindowTitle("Bill " + bill.billId);
    resize(700, 480);

    QTextEdit* billTextView = new QTextEdit;
    billTextView->setReadOnly(true);
    billTextView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    billTextView->setLineWrapMode(QTextEdit::NoWrap);
    billTextView->setPlainText(BillFormatter::screenText(bill));

    QPushButton* closeButton = new QPushButton("Close");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(billTextView, 1);
    layout->addWidget(closeButton);
}
