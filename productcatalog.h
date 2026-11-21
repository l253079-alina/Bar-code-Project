#pragma once

#include "orderitem.h"
#include "product.h"
#include "productrepository.h"
#include "standardnames.h"

#include <QString>

class ProductCatalog {
public:
    explicit ProductCatalog(ProductRepository& repositoryToUse);

    bool reload(QString& problemDescription);
    optional<Product> findByBarcode(const QString& barcode) const;
    optional<Product> findByProductId(const QString& productId) const;
    const vector<Product>& allProducts() const;
    QString createNextProductId() const;
    bool addProduct(const Product& newProduct, QString& problemDescription);
    bool updateProduct(const Product& changedProduct, QString& problemDescription);
    bool reduceStock(const vector<OrderItem>& soldItems, QString& problemDescription);

private:
    ProductRepository& productRepository;
    vector<Product> products;

    bool barcodeIsUsedByAnotherProduct(const QString& barcode, const QString& productId) const;
};