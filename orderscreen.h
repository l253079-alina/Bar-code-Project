#pragma once

#include "checkoutservice.h"
#include "product.h"
#include "productcatalog.h"
#include "shoppingcart.h"
#include "standardnames.h"

#include <QWidget>

class QDoubleSpinBox;
class QLabel;
class QTableWidget;
class ScanPanel;

class OrderScreen : public QWidget {
public:
    OrderScreen(ProductCatalog& catalog, CheckoutService& checkout, QWidget* parent = nullptr);
    void stopScanning();
    void handleScannedBarcode(const QString& scannedBarcode);

private:
    ProductCatalog& productCatalog;
    CheckoutService& checkoutService;
    ShoppingCart shoppingCart;
    optional<Product> lastScannedProduct;

    ScanPanel* scanPanel = nullptr;
    QDoubleSpinBox* quantitySpinBox = nullptr;
    QTableWidget* orderTable = nullptr;
    QLabel* totalsLabel = nullptr;
    QLabel* messageLabel = nullptr;

    void refreshOrderTable();
    void refreshTotals();
    void showMessage(const QString& messageText);
    void showDescription();
    void removeSelectedItem();
    void clearOrder();
    void generateBill();
};
