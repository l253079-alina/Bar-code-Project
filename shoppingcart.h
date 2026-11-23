#pragma once

#include "orderitem.h"
#include "standardnames.h"

#include <QString>

class ShoppingCart {
public:
    static double quantityStepFor(const Product& product);

    bool addProduct(const Product& product, double requiredQuantity, QString& rejectionReason);
    bool changeQuantityAt(int itemIndex, double quantityChange, QString& rejectionReason);
    void removeItemAt(int itemIndex);
    void setReservedItems(const vector<OrderItem>& reservedItems);
    void clear();
    bool isEmpty() const;
    const vector<OrderItem>& items() const;

private:
    int findItemIndexByProductId(const QString& productId) const;
    double reservedQuantityFor(const QString& productId) const;

    vector<OrderItem> orderItems;
    map<QString, double> reservedQuantities;
};
