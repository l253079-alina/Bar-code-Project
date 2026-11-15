#include "fileproductrepository.h"

#include "productlineparser.h"

#include <QFile>
#include <QStringList>
#include <QTextStream>

FileProductRepository::FileProductRepository(const QString& productFilePathToRead) : productFilePath(productFilePathToRead){
}

bool FileProductRepository::loadProducts(vector<Product>& products, QString& problemDescription){
    products.clear();
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
        if (line.trimmed().isEmpty()) {
            continue;
        }

        Product product;
        if (ProductLineParser::parseLine(line, product)) {
            products.push_back(product);
        }
        else {
            skippedLineNumbers.append(QString::number(currentLineNumber));
        }
    }

    if (!skippedLineNumbers.isEmpty()) {
        problemDescription = "These lines of Product.txt could not be read and were skipped: " + skippedLineNumbers.join(", ");
    }
    return true;
}