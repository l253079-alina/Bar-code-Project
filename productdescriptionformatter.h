#pragma once
#include "product.h"
#include <QString>

class ProductDescriptionFormatter {
public:
    static QString describe(const Product& product);
};
