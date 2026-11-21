#include "fileproductrepository.h"

#include "filefieldformatter.h"
#include "productlineparser.h"
#include <QFile>
#include <QSaveFile>
#include <QStringList>
#include <QTextStream>

FileProductRepository::FileProductRepository(const QString& productFilePathToRead)
    : productFilePath(productFilePathToRead)
{
}

bool FileProductRepository::loadProducts(vector<Product>& products, QString& problemDescription){
    products.clear();
    unreadableLines.clear();
    problemDescription.clear();

    QFile productFile(productFilePath);
    if (!productFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        problemDescription = "Could not open " + productFilePath;
        return false;
    }

    QTextStream productStream(&productFile);
    QStringList skippedLineNumbers;
    int currentLineNumber = 0;
    while (!productStream.atEnd()) {
        const QString line = productStream.readLine();
        ++currentLineNumber;
        if (line.trimmed().isEmpty())
            continue;

        Product product;
        if (ProductLineParser::parseLine(line, product)) {
            products.push_back(product);
        }
        else {
            skippedLineNumbers.append(QString::number(currentLineNumber));
            unreadableLines.append(line.trimmed());
        }
    }

    if (!skippedLineNumbers.isEmpty())
        problemDescription = "These lines of Product.txt could not be read and were skipped: "
        + skippedLineNumbers.join(", ");
    return true;
}

bool FileProductRepository::saveProducts(const vector<Product>& products, QString& problemDescription){
    QSaveFile productFile(productFilePath);
    if (!productFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        problemDescription = "Could not write to " + productFilePath;
        return false;
    }

    QTextStream productStream(&productFile);
    for (const Product& product : products) {
        QStringList fields;
        fields << product.productId << product.barcode << product.acronym << product.name << product.measurementUnit
            << FileFieldFormatter::money(product.unitPrice) << FileFieldFormatter::money(product.taxPerUnit) << FileFieldFormatter::money(product.discountPerUnit) 
            << FileFieldFormatter::quantity(product.availableQuantity) << product.barcodeFilePath;
        productStream << FileFieldFormatter::lineWithEndMarker(fields) << Qt::endl;
    }
    for (const QString& unreadableLine : unreadableLines)
        productStream << unreadableLine << Qt::endl;
    productStream.flush();

    if (!productFile.commit()) {
        problemDescription = "Could not write to " + productFilePath;
        return false;
    }
    return true;
}