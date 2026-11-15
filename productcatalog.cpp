#include "productcatalog.h"
#include <vector>


ProductCatalog::ProductCatalog(ProductRepository& repositoryToUse)
    : productRepository(repositoryToUse){
}

bool ProductCatalog::reload(QString& problemDescription){
    return productRepository.loadProducts(products, problemDescription);
}

optional<Product> ProductCatalog::findByBarcode(const QString& barcode) const
{
    const QString trimmedBarcode = barcode.trimmed();
    for (const Product& product : products) {
        if (product.barcode == trimmedBarcode)
            return product;
    }
    return nullopt;
}

optional<Product> ProductCatalog::findByProductId(const QString& productId) const
{
    for (const Product& product : products) {
        if (product.productId == productId)
            return product;
    }
    return nullopt;
}

const vector<Product>& ProductCatalog::allProducts() const
{
    return products;
}