#include "productinfoscreen.h"

#include "productdescriptionformatter.h"
#include "scanpanel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>

ProductInfoScreen::ProductInfoScreen(ProductCatalog& catalog, QWidget* parent)
    : QWidget(parent), productCatalog(catalog)
{
    scanPanel = new ScanPanel;

    descriptionView = new QTextEdit;
    descriptionView->setReadOnly(true);
    descriptionView->setPlaceholderText("Scan a product barcode to see its description here.");

    QVBoxLayout* descriptionLayout = new QVBoxLayout;
    descriptionLayout->addWidget(new QLabel("Product description"));
    descriptionLayout->addWidget(descriptionView, 1);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(scanPanel, 1);
    layout->addLayout(descriptionLayout, 1);

    connect(scanPanel, &ScanPanel::barcodeScanned, this,
            [this](const QString& scannedBarcode) { showProductForBarcode(scannedBarcode); });
}

void ProductInfoScreen::stopScanning()
{
    scanPanel->stopScanning();
}

void ProductInfoScreen::showProductForBarcode(const QString& scannedBarcode)
{
    const optional<Product> foundProduct = productCatalog.findByBarcode(scannedBarcode);
    if (!foundProduct.has_value()) {
        descriptionView->setPlainText("No product found for barcode " + scannedBarcode);
        return;
    }
    descriptionView->setPlainText(ProductDescriptionFormatter::describe(*foundProduct));
}
