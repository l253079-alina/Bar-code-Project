#include "productrecordformatter.h"
#include "filefieldformatter.h"

static const int categoryIdLength = 2;

QString ProductRecordFormatter::line(const Product& product){
    QString categoryId = product.categoryId;
    if (categoryId.isEmpty())
        categoryId = product.barcode.left(categoryIdLength);

    QString barcodeFilePath = product.barcodeFilePath;
    if (barcodeFilePath.trimmed().isEmpty())
        barcodeFilePath = "Barcodes/" + product.barcode + ".png";

    QString cleanedName = product.name;
    cleanedName.replace(QChar('#'), QChar(' '));
    cleanedName = cleanedName.simplified();

    QStringList fields;
    fields << product.productId << product.barcode << categoryId << product.acronym
        << cleanedName << product.measurementUnit << FileFieldFormatter::money(product.unitPrice)
        << FileFieldFormatter::money(product.taxPerUnit) << FileFieldFormatter::money(product.discountPerUnit) << FileFieldFormatter::quantity(product.availableQuantity) << barcodeFilePath;
    return FileFieldFormatter::lineWithEndMarker(fields);
}