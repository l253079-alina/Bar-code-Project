#pragma once

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

private:
    ProductRepository& productRepository;
    vector<Product> products;
};