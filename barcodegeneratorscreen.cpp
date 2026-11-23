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
static const int categoryIdLength = 2;

BarcodeGeneratorScreen::BarcodeGeneratorScreen(ProductCatalog& catalog, CategoryCatalog& categories, const QString& dataFolderToUse, QWidget* parent)
    : QWidget(parent), productCatalog(catalog), categoryCatalog(categories), dataFolderPath(dataFolderToUse) {
    productComboBox = new QComboBox;
    categoryComboBox = new QComboBox;

    numberLineEdit = new QLineEdit;
    numberLineEdit->setPlaceholderText("6 digits: 2 digit category id and a 4 digit number");
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
    layout->addWidget(new QLabel("Category (the first two digits of the barcode)"));
    layout->addWidget(categoryComboBox);
    layout->addWidget(new QLabel("Barcode number"));
    layout->addLayout(numberLayout);
    layout->addWidget(barcodeView, 1);
    layout->addWidget(messageLabel);

    reloadData();

    connect(productComboBox, &QComboBox::currentIndexChanged, this, [this] { useSelectedProduct(); });
    connect(categoryComboBox, &QComboBox::activated, this, [this] { useSelectedCategory(); });
    connect(generateButton, &QPushButton::clicked, this, [this] { generateBarcode(); });
    connect(saveButton, &QPushButton::clicked, this, [this] { saveBarcode(); });
    connect(numberLineEdit, &QLineEdit::returnPressed, this, [this] { generateBarcode(); });
}

void BarcodeGeneratorScreen::reloadData(){
    productComboBox->blockSignals(true);
    productComboBox->clear();
    productComboBox->addItem("Type a number manually");
    for (const Product& product : productCatalog.allProducts()) {
        productComboBox->addItem(product.acronym + " - " + product.name);
        const int newItemIndex = productComboBox->count() - 1;
        productComboBox->setItemData(newItemIndex, product.barcode, productBarcodeRole);
        productComboBox->setItemData(newItemIndex, product.barcodeFilePath, productFilePathRole);
    }
    productComboBox->blockSignals(false);

    categoryComboBox->clear();
    categoryComboBox->addItem("Choose a category to start a new number");
    for (const Category& category : categoryCatalog.allCategories())
        categoryComboBox->addItem(category.categoryId + " - " + category.name, category.categoryId);
}

void BarcodeGeneratorScreen::useSelectedProduct(){
    if (productComboBox->currentIndex() <= 0)
        return;
    const QString productBarcode = productComboBox->currentData(productBarcodeRole).toString();
    numberLineEdit->setText(productBarcode);

    const int categoryIndex = categoryComboBox->findData(productBarcode.left(categoryIdLength));
    if (categoryIndex >= 0)
        categoryComboBox->setCurrentIndex(categoryIndex);
    generateBarcode();
}

void BarcodeGeneratorScreen::useSelectedCategory(){
    if (categoryComboBox->currentIndex() <= 0)
        return;
    productComboBox->blockSignals(true);
    productComboBox->setCurrentIndex(0);
    productComboBox->blockSignals(false);

    const QString chosenCategoryId = categoryComboBox->currentData().toString();
    QString nextBarcode;
    QString problemDescription;
    if (!productCatalog.nextBarcodeForCategory(chosenCategoryId, nextBarcode, problemDescription)) {
        messageLabel->setText(problemDescription);
        return;
    }
    numberLineEdit->setText(nextBarcode);
    generateBarcode();
}

void BarcodeGeneratorScreen::generateBarcode(){
    const QString barcodeNumber = numberLineEdit->text();
    const bool numberIsValid = barcodeNumber.size() == 6 && barcodeNumber[0] != QChar('0');
    if (!numberIsValid) {
        messageLabel->setText("Enter exactly 6 digits. The first digit must not be zero.");
        return;
    }

    const bool enteredManually = productComboBox->currentIndex() <= 0;
    if (enteredManually && !categoryCatalog.findById(barcodeNumber.left(categoryIdLength)).has_value()) {
        messageLabel->setText("No category has the id " + barcodeNumber.left(categoryIdLength)
            + ". Add the category first, because the first two digits of a barcode are its category id.");
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

QString BarcodeGeneratorScreen::chooseSavePath() const{
    QString productFilePath;
    if (productComboBox->currentIndex() > 0)
        productFilePath = productComboBox->currentData(productFilePathRole).toString();

    if (!productFilePath.isEmpty())
        return QDir(dataFolderPath).absoluteFilePath(productFilePath);

    return QFileDialog::getSaveFileName(const_cast<BarcodeGeneratorScreen*>(this), "Save barcode",
        "barcode.png", "PNG (*.png)");
}

void BarcodeGeneratorScreen::saveBarcode(){
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