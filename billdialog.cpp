#include "billdialog.h"

#include "billformatter.h"
#include "billpdfwriter.h"

#include <QFileDialog>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

BillDialog::BillDialog(const Bill& bill, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Bill " + bill.billId);
    resize(700, 520);

    QTextEdit* billTextView = new QTextEdit;
    billTextView->setReadOnly(true);
    billTextView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    billTextView->setLineWrapMode(QTextEdit::NoWrap);
    billTextView->setPlainText(BillFormatter::screenText(bill));

    QPushButton* pdfButton = new QPushButton("Save as PDF...");
    QPushButton* closeButton = new QPushButton("Close");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(pdfButton, &QPushButton::clicked, this, [this, bill] {
        const QString pdfPath = QFileDialog::getSaveFileName(this, "Save bill as PDF","Bill_" + bill.billId + ".pdf", "PDF (*.pdf)");
        if (pdfPath.isEmpty())
            return;

        QString problemDescription;
        if (!BillPdfWriter::savePdf(bill, pdfPath, problemDescription)) {
            QMessageBox::critical(this, "Save as PDF", problemDescription);
            return;
        }
        QMessageBox::information(this, "Save as PDF", "The bill was saved to " + pdfPath);
    });

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(pdfButton);
    buttonsLayout->addWidget(closeButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(billTextView, 1);
    layout->addLayout(buttonsLayout);
}
