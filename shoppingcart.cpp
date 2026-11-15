#include "shoppingcart.h"

#include "orderconstants.h"

#include <cmath>

static const double quantityTolerance = 0.0000001;

int ShoppingCart::findItemIndexByProductId(const QString& productId) const
{
    for (size_t itemIndex = 0; itemIndex < orderItems.size(); ++itemIndex) {
        if (orderItems[itemIndex].product.productId == productId)
            return static_cast<int>(itemIndex);
    }
    return -1;
}

bool ShoppingCart::addProduct(const Product& product, double requiredQuantity, QString& rejectionReason)
{
    if (requiredQuantity <= 0.0) {
        rejectionReason = "The quantity must be greater than zero.";
        return false;
    }

    bool productIsMeasuredByCount;
    if (product.measurementUnit.toLower() == OrderConstants::measurementUnitCount)
        productIsMeasuredByCount = true;
    else
        productIsMeasuredByCount = false;
    if (productIsMeasuredByCount && requiredQuantity != floor(requiredQuantity)) {
        rejectionReason = product.name + " is sold by count, so the quantity must be a whole number.";
        return false;
    }

    const int existingItemIndex = findItemIndexByProductId(product.productId);
    double quantityAlreadyInOrder = 0.0;
    if (existingItemIndex >= 0)
        quantityAlreadyInOrder = orderItems[existingItemIndex].quantity;

    const double totalRequiredQuantity = quantityAlreadyInOrder + requiredQuantity;
    if (totalRequiredQuantity > product.availableQuantity + quantityTolerance) {
        rejectionReason = QString("Not enough stock for %1. Available: %2, already in this order: %3, requested now: %4.")
            .arg(product.name)
            .arg(product.availableQuantity, 0, 'g', 10)
            .arg(quantityAlreadyInOrder, 0, 'g', 10)
            .arg(requiredQuantity, 0, 'g', 10);
        return false;
    }

    if (existingItemIndex >= 0) {
        orderItems[existingItemIndex].quantity = totalRequiredQuantity;
    } else {
        OrderItem newItem;
        newItem.product = product;
        newItem.quantity = requiredQuantity;
        orderItems.push_back(newItem);
    }
    return true;
}

void ShoppingCart::removeItemAt(int itemIndex)
{
    if (itemIndex < 0 || itemIndex >= static_cast<int>(orderItems.size()))
        return;
    orderItems.erase(orderItems.begin() + itemIndex);
}

void ShoppingCart::clear()
{
    orderItems.clear();
}

bool ShoppingCart::isEmpty() const
{
    return orderItems.empty();
}

const vector<OrderItem>& ShoppingCart::items() const
{
    return orderItems;
}
