#pragma once

#include "productrepository.h"

class FileProductRepository : public ProductRepository {
public:
    explicit FileProductRepository(const QString& productFilePathToRead);
    bool loadProducts(vector<Product>& products, QString& problemDescription) override;

private:
    QString productFilePath;
};