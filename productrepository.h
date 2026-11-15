#pragma once

#include "product.h"
#include "standardnames.h"

#include <QString>

class ProductRepository {
public:
    virtual ~ProductRepository() = default;
    virtual bool loadProducts(vector<Product>& products, QString& problemDescription) = 0;
};