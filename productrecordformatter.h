#pragma once
#include "product.h"
#include <QString>

class ProductRecordFormatter {
public:
    static QString line(const Product& product);
};