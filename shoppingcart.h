#pragma once

#include "orderitem.h"
#include "standardnames.h"

#include <QString>

class ShoppingCart {
public:
    bool addProduct(const Product& product, double requiredQuantity, QString& rejectionReason);
    void removeItemAt(int itemIndex);
    void clear();
    bool isEmpty() const;
    const vector<OrderItem>& items() const;

private:
    int findItemIndexByProductId(const QString& productId) const;

    vector<OrderItem> orderItems;
};
