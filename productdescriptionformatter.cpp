#include "productdescriptionformatter.h"
#include "filefieldformatter.h"
#include "productrecordformatter.h"
#include <QStringList>

QString ProductDescriptionFormatter::describe(const Product& product)
{
    QStringList lines;
    lines << "Product ID: " + product.productId;
    lines << "Barcode: " + product.barcode;
    lines << "Category ID: " + product.categoryId;
    lines << "Acronym: " + product.acronym;
    lines << "Name: " + product.name;
    lines << "Measurement unit: " + product.measurementUnit;
    lines << "Unit price: PKR " + FileFieldFormatter::money(product.unitPrice);
    lines << "Tax: " + ProductRecordFormatter::amountText(product.taxValue, product.taxIsPercentage) + " (PKR " + FileFieldFormatter::money(product.taxAmountPerUnit()) + " per unit)";
    lines << "Discount: " + ProductRecordFormatter::amountText(product.discountValue, product.discountIsPercentage) + " (PKR " + FileFieldFormatter::money(product.discountAmountPerUnit()) + " per unit)";
    lines << "Price to pay per unit: PKR " + FileFieldFormatter::money(product.priceToPayPerUnit());
    lines << "Available quantity: " + FileFieldFormatter::quantity(product.availableQuantity);
    lines << "Barcode image file: " + product.barcodeFilePath;
    return lines.join(QChar('\n'));
}
