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
    optional<Product> findByAcronym(const QString& acronym) const;
    const vector<Product>& allProducts() const;

    bool acronymInUse(const QString& acronym, const QString& exceptProductId) const;
    bool categoryInUse(const QString& categoryId) const;
    QString createNextProductId() const;
    bool nextBarcodeForCategory(const QString& categoryId, QString& barcode, QString& problemDescription) const;

    bool addProduct(const Product& newProduct, QString& problemDescription);
    bool updateProduct(const Product& changedProduct, QString& problemDescription);
    bool setAvailableQuantity(const QString& productId, double newQuantity, QString& problemDescription);
    bool adjustStock(const map<QString, double>& quantityToDeductByProductId, QString& problemDescription);

private:
    ProductRepository& productRepository;
    vector<Product> products;

    bool saveChangedProducts(const vector<Product>& changedProducts, QString& problemDescription);
};