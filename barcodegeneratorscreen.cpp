#include "barcodegeneratorscreen.h"
#include "barcodeUtils.h"

#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>

static const int productBarcodeRole = Qt::UserRole;
static const int productFilePathRole = Qt::UserRole + 1;

BarcodeGeneratorScreen::BarcodeGeneratorScreen(ProductCatalog& catalog, const QString& dataFolderToUse, QWidget* parent)
    : QWidget(parent), productCatalog(catalog), dataFolderPath(dataFolderToUse)
{
    productComboBox = new QComboBox;

    numberLineEdit = new QLineEdit;
    numberLineEdit->setPlaceholderText("6 digits, the first digit is not zero");
    numberLineEdit->setMaxLength(6);
    numberLineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[1-9]\\d{0,5}"), this));

    QPushButton* generateButton = new QPushButton("Generate barcode");
    QPushButton* saveButton = new QPushButton("Save barcode image");

    barcodeView = new QLabel("The generated barcode appears here");
    barcodeView->setAlignment(Qt::AlignCenter);
    barcodeView->setMinimumHeight(220);
    barcodeView->setFrameShape(QFrame::StyledPanel);

    messageLabel = new QLabel;
    messageLabel->setWordWrap(true);

    QHBoxLayout* numberLayout = new QHBoxLayout;
    numberLayout->addWidget(numberLineEdit, 1);
    numberLayout->addWidget(generateButton);
    numberLayout->addWidget(saveButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Product"));
    layout->addWidget(productComboBox);
    layout->addWidget(new QLabel("Barcode number"));
    layout->addLayout(numberLayout);
    layout->addWidget(barcodeView, 1);
    layout->addWidget(messageLabel);

    reloadProducts();

    connect(productComboBox, &QComboBox::currentIndexChanged, this, [this] { useSelectedProduct(); });
    connect(generateButton, &QPushButton::clicked, this, [this] { generateBarcode(); });
    connect(saveButton, &QPushButton::clicked, this, [this] { saveBarcode(); });
    connect(numberLineEdit, &QLineEdit::returnPressed, this, [this] { generateBarcode(); });
}

void BarcodeGeneratorScreen::reloadProducts()
{
    productComboBox->clear();
    productComboBox->addItem("Type a number manually");

    for (const Product& product : productCatalog.allProducts()) {
        productComboBox->addItem(product.acronym + " - " + product.name);

        const int newItemIndex = productComboBox->count() - 1;

        productComboBox->setItemData(newItemIndex, product.barcode, productBarcodeRole);
        productComboBox->setItemData(newItemIndex, product.barcodeFilePath, productFilePathRole);
    }
}

void BarcodeGeneratorScreen::useSelectedProduct()
{
    if (productComboBox->currentIndex() <= 0)
        return;

    numberLineEdit->setText(productComboBox->currentData(productBarcodeRole).toString());
    generateBarcode();
}

void BarcodeGeneratorScreen::generateBarcode()
{
    const QString barcodeNumber = numberLineEdit->text();

    const bool numberIsValid = barcodeNumber.size() == 6 && barcodeNumber[0] != QChar('0');

    if (!numberIsValid) {
        messageLabel->setText("Enter exactly 6 digits. The first digit must not be zero.");
        return;
    }

    generatedBarcode = makeBarcodeImage(barcodeNumber);

    if (generatedBarcode.isNull()) {
        messageLabel->setText("The barcode could not be generated for that number.");
        return;
    }

    barcodeView->setPixmap(QPixmap::fromImage(generatedBarcode).scaled(
        barcodeView->size(), Qt::KeepAspectRatio, Qt::FastTransformation));

    messageLabel->setText("Generated a Code 128 barcode for " + barcodeNumber);
}

QString BarcodeGeneratorScreen::chooseSavePath() const
{
    QString productFilePath;

    if (productComboBox->currentIndex() > 0)
        productFilePath = productComboBox->currentData(productFilePathRole).toString();

    if (!productFilePath.isEmpty())
        return QDir(dataFolderPath).absoluteFilePath(productFilePath);

    return QFileDialog::getSaveFileName(const_cast<BarcodeGeneratorScreen*>(this), "Save barcode",
        "barcode.png", "PNG (*.png)");
}

void BarcodeGeneratorScreen::saveBarcode()
{
    if (generatedBarcode.isNull())
        generateBarcode();

    if (generatedBarcode.isNull())
        return;

    const QString savePath = chooseSavePath();

    if (savePath.isEmpty())
        return;

    QFileInfo(savePath).absoluteDir().mkpath(".");

    if (!generatedBarcode.save(savePath)) {
        messageLabel->setText("Could not save the file " + savePath);
        return;
    }

    messageLabel->setText("Saved the barcode image to " + savePath);
}
