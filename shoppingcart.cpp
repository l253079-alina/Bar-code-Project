#include "shoppingcart.h"

#include "orderconstants.h"

#include <cmath>

static const double quantityTolerance = 0.0000001;
static const double countProductStep = 1.0;
static const double weightProductStep = 0.5;

static bool isMeasuredByCount(const Product& product)
{
    return product.measurementUnit.toLower() == OrderConstants::measurementUnitCount;
}

static bool isWholeNumber(double value)
{
    return fabs(value - floor(value + 0.5)) < quantityTolerance;
}

static QString stockRejectionReason(const Product& product, double reservedQuantity, double quantityAlreadyInOrder,
                                    double requestedQuantity)
{
    const double totalRequiredQuantity = quantityAlreadyInOrder + requestedQuantity;
    const double quantityOfferedToThisOrder = product.availableQuantity + reservedQuantity;
    if (totalRequiredQuantity <= quantityOfferedToThisOrder + quantityTolerance)
        return QString();

    return QString("Not enough stock for %1. Available: %2, already in this order: %3, requested now: %4.")
        .arg(product.name)
        .arg(quantityOfferedToThisOrder, 0, 'g', 10)
        .arg(quantityAlreadyInOrder, 0, 'g', 10)
        .arg(requestedQuantity, 0, 'g', 10);
}

double ShoppingCart::quantityStepFor(const Product& product)
{
    if (isMeasuredByCount(product))
        return countProductStep;
    return weightProductStep;
}

double ShoppingCart::reservedQuantityFor(const QString& productId) const
{
    const auto foundQuantity = reservedQuantities.find(productId);
    if (foundQuantity == reservedQuantities.end())
        return 0.0;
    return foundQuantity->second;
}

void ShoppingCart::setReservedItems(const vector<OrderItem>& reservedItems)
{
    reservedQuantities.clear();
    for (const OrderItem& item : reservedItems)
        reservedQuantities[item.product.productId] += item.quantity;
}

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

    if (isMeasuredByCount(product) && !isWholeNumber(requiredQuantity)) {
        rejectionReason = product.name + " is sold by count, so the quantity must be a whole number.";
        return false;
    }

    const int existingItemIndex = findItemIndexByProductId(product.productId);
    double quantityAlreadyInOrder = 0.0;
    if (existingItemIndex >= 0)
        quantityAlreadyInOrder = orderItems[existingItemIndex].quantity;

    rejectionReason = stockRejectionReason(product, reservedQuantityFor(product.productId), quantityAlreadyInOrder,
                                           requiredQuantity);
    if (!rejectionReason.isEmpty())
        return false;

    if (existingItemIndex >= 0) {
        orderItems[existingItemIndex].quantity = quantityAlreadyInOrder + requiredQuantity;
    } else {
        OrderItem newItem;
        newItem.product = product;
        newItem.quantity = requiredQuantity;
        orderItems.push_back(newItem);
    }
    return true;
}

bool ShoppingCart::changeQuantityAt(int itemIndex, double quantityChange, QString& rejectionReason)
{
    if (itemIndex < 0 || itemIndex >= static_cast<int>(orderItems.size())) {
        rejectionReason = "That product is not in the order.";
        return false;
    }

    OrderItem& item = orderItems[itemIndex];
    const double newQuantity = item.quantity + quantityChange;
    if (newQuantity <= quantityTolerance) {
        removeItemAt(itemIndex);
        return true;
    }

    if (isMeasuredByCount(item.product) && !isWholeNumber(newQuantity)) {
        rejectionReason = item.product.name + " is sold by count, so the quantity must be a whole number.";
        return false;
    }

    if (quantityChange > 0.0) {
        rejectionReason = stockRejectionReason(item.product, reservedQuantityFor(item.product.productId), item.quantity,
                                               quantityChange);
        if (!rejectionReason.isEmpty())
            return false;
    }

    item.quantity = newQuantity;
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
    reservedQuantities.clear();
}

bool ShoppingCart::isEmpty() const
{
    return orderItems.empty();
}

const vector<OrderItem>& ShoppingCart::items() const
{
    return orderItems;
}
