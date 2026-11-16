#include "productdescriptionformatter.h"
#include "filefieldformatter.h"
#include <QStringList>

QString ProductDescriptionFormatter::describe(const Product& product)
{
    QStringList lines;
    lines << "Product ID: " + product.productId;
    lines << "Barcode: " + product.barcode;
    lines << "Acronym: " + product.acronym;
    lines << "Name: " + product.name;
    lines << "Measurement unit: " + product.measurementUnit;
    lines << "Unit price: PKR " + FileFieldFormatter::money(product.unitPrice);
    lines << "Tax per unit: PKR " + FileFieldFormatter::money(product.taxPerUnit);
    lines << "Discount per unit: PKR " + FileFieldFormatter::money(product.discountPerUnit);
    lines << "Available quantity: " + FileFieldFormatter::quantity(product.availableQuantity);
    lines << "Barcode image file: " + product.barcodeFilePath;
    return lines.join(QChar('\n'));
}
