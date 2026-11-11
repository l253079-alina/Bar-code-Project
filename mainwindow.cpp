#include "mainwindow.h"

#include "barcodeutils.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>

MainWindow::MainWindow()
{
    setWindowTitle("Barcode <-> 6-digit number");

    preview = new QLabel("Open an image to decode its barcode");
    preview->setAlignment(Qt::AlignCenter);
    preview->setMinimumSize(520, 380);
    preview->setFrameShape(QFrame::StyledPanel);

    auto* openBtn = new QPushButton("Open image file...");

    resultLabel = new QLabel("Result: -");
    resultLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    resultLabel->setWordWrap(true);

    barcodeView = new QLabel("Generated barcode appears here");
    barcodeView->setAlignment(Qt::AlignCenter);
    barcodeView->setMinimumHeight(170);
    barcodeView->setFrameShape(QFrame::StyledPanel);

    auto* copyBtn = new QPushButton("Copy 6-digit number");

    numberEdit = new QLineEdit;
    numberEdit->setPlaceholderText("Enter 6 digits");
    numberEdit->setMaxLength(6);
    numberEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d{0,6}"), this));
    auto* genBtn = new QPushButton("Generate barcode");
    auto* saveBtn = new QPushButton("Save barcode PNG...");

    auto* row1 = new QHBoxLayout;
    row1->addWidget(openBtn);

    auto* row2 = new QHBoxLayout;
    row2->addWidget(numberEdit);
    row2->addWidget(genBtn);
    row2->addWidget(saveBtn);

    auto* layout = new QVBoxLayout;
    layout->addWidget(preview, 1);
    layout->addLayout(row1);
    layout->addWidget(resultLabel);
    layout->addWidget(barcodeView);
    layout->addWidget(copyBtn);
    layout->addLayout(row2);

    auto* central = new QWidget;
    central->setLayout(layout);
    setCentralWidget(central);

    connect(openBtn, &QPushButton::clicked, this, [this] { openFile(); });
    connect(copyBtn, &QPushButton::clicked, this, [this] {
        if (!lastNumber.isEmpty())
            QApplication::clipboard()->setText(lastNumber);
        });
    connect(genBtn, &QPushButton::clicked, this, [this] { generate(); });
    connect(saveBtn, &QPushButton::clicked, this, [this] { saveBarcode(); });
    connect(numberEdit, &QLineEdit::returnPressed, this, [this] { generate(); });
}

void MainWindow::showImage(const QImage& img)
{
    preview->setPixmap(QPixmap::fromImage(img).scaled(preview->size(), Qt::KeepAspectRatio,
        Qt::SmoothTransformation));
}

void MainWindow::showGenerated(const QString& n)
{
    generated = makeBarcodeImage(n);
    if (generated.isNull())
        return;
    barcodeView->setPixmap(QPixmap::fromImage(generated).scaled(
        barcodeView->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
}

void MainWindow::showResult(const QString& text)
{
    lastNumber = toSixDigits(text);
    numberEdit->setText(lastNumber);
    showGenerated(lastNumber);
    resultLabel->setText(QString("Barcode (Code 128): %1   ->   6-digit number: %2")
        .arg(text, lastNumber));
}

void MainWindow::openFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Open barcode image", {}, "Images (*.png *.jpg *.jpeg *.bmp *.gif *.webp)");
    if (path.isEmpty())
        return;
    QImage img(path);
    if (img.isNull()) {
        QMessageBox::warning(this, "Error", "Could not load that image.");
        return;
    }
    showImage(img);
    if (auto text = decodeBarcode(img))
        showResult(*text);
    else
        resultLabel->setText("No Code 128 barcode found in that image.");
}

void MainWindow::generate()
{
    const QString n = numberEdit->text();
    if (n.size() != 6) {
        QMessageBox::information(this, "Input", "Please enter exactly 6 digits.");
        return;
    }
    lastNumber = n;
    showGenerated(n);
    resultLabel->setText(QString("Generated Code 128 barcode for %1").arg(n));
}

void MainWindow::saveBarcode()
{
    if (generated.isNull())
        generate();
    if (generated.isNull())
        return;
    const QString path = QFileDialog::getSaveFileName(this, "Save barcode", "barcode.png",
        "PNG (*.png)");
    if (!path.isEmpty() && !generated.save(path))
        QMessageBox::warning(this, "Error", "Could not save the file.");
}
