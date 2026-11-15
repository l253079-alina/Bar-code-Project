#pragma once

#include "product.h"

struct OrderItem {
    Product product;
    double quantity = 0.0;
};
