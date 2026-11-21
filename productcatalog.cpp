#include "productcatalog.h"

#include <algorithm>
#include <cmath>

ProductCatalog::ProductCatalog(ProductRepository& repositoryToUse)
    : productRepository(repositoryToUse)
{
}

bool ProductCatalog::reload(QString& problemDescription){
    return productRepository.loadProducts(products, problemDescription);
}

optional<Product> ProductCatalog::findByBarcode(const QString& barcode) const{
    const QString trimmedBarcode = barcode.trimmed();
    for (const Product& product : products) {
        if (product.barcode == trimmedBarcode)
            return product;
    }
    return nullopt;
}

optional<Product> ProductCatalog::findByProductId(const QString& productId) const{
    for (const Product& product : products) {
        if (product.productId == productId)
            return product;
    }
    return nullopt;
}

const vector<Product>& ProductCatalog::allProducts() const{
    return products;
}

QString ProductCatalog::createNextProductId() const{
    int highestProductNumber = 0;
    for (const Product& product : products) {
        bool conversionWorked = false;
        const int productNumber = product.productId.toInt(&conversionWorked);
        if (conversionWorked && productNumber > highestProductNumber)
            highestProductNumber = productNumber;
    }
    return QString::number(highestProductNumber + 1);
}

bool ProductCatalog::barcodeIsUsedByAnotherProduct(const QString& barcode, const QString& productId) const{
    for (const Product& product : products) {
        if (product.barcode == barcode && product.productId != productId)
            return true;
    }
    return false;
}

bool ProductCatalog::addProduct(const Product& newProduct, QString& problemDescription){
    if (barcodeIsUsedByAnotherProduct(newProduct.barcode, newProduct.productId)) {
        problemDescription = "Another product already uses the barcode " + newProduct.barcode + ".";
        return false;
    }

    products.push_back(newProduct);
    if (!productRepository.saveProducts(products, problemDescription)) {
        products.pop_back();
        return false;
    }
    return true;
}

bool ProductCatalog::updateProduct(const Product& changedProduct, QString& problemDescription){
    if (barcodeIsUsedByAnotherProduct(changedProduct.barcode, changedProduct.productId)) {
        problemDescription = "Another product already uses the barcode " + changedProduct.barcode + ".";
        return false;
    }

    for (Product& product : products) {
        if (product.productId != changedProduct.productId)
            continue;

        const Product productBeforeChange = product;
        product = changedProduct;
        if (!productRepository.saveProducts(products, problemDescription)) {
            product = productBeforeChange;
            return false;
        }
        return true;
    }

    problemDescription = "The product to update was not found.";
    return false;
}

bool ProductCatalog::reduceStock(const vector<OrderItem>& soldItems, QString& problemDescription){
    const vector<Product> productsBeforeSale = products;
    for (const OrderItem& soldItem : soldItems) {
        for (Product& product : products) {
            if (product.productId != soldItem.product.productId)
                continue;

            const double remainingQuantity = product.availableQuantity - soldItem.quantity;
            const double roundedRemainingQuantity = round(remainingQuantity * 1000.0) / 1000.0;
            product.availableQuantity = max(0.0, roundedRemainingQuantity);
            break;
        }
    }

    if (!productRepository.saveProducts(products, problemDescription)) {
        products = productsBeforeSale;
        return false;
    }
    return true;
}