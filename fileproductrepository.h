#pragma once

#include "productrepository.h"
#include <QStringList>

class FileProductRepository : public ProductRepository {
public:
    explicit FileProductRepository(const QString& productFilePathToRead);
    bool loadProducts(vector<Product>& products, QString& problemDescription) override;
    bool saveProducts(const vector<Product>& products, QString& problemDescription) override;

private:
    QString productFilePath;
    QStringList unreadableLines;
};