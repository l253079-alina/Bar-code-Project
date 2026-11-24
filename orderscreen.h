#pragma once

#include "checkoutservice.h"
#include "existingorderinfo.h"
#include "heldorderservice.h"
#include "managerapprovalservice.h"
#include "product.h"
#include "productcatalog.h"
#include "shoppingcart.h"
#include "standardnames.h"

#include <QWidget>

class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QTableWidget;
class ScanPanel;

class OrderScreen : public QWidget {
public:
    OrderScreen(ProductCatalog& catalog, CheckoutService& checkout, HeldOrderService& heldOrders, ManagerApprovalService& approvalService, QWidget* parent = nullptr);
    void stopScanning();
    void handleScannedBarcode(const QString& scannedBarcode);
    void changeQuantityOfRow(int rowIndex, int direction);
    void holdOrder();
    void openOrder(const QString& orderId);

private:
    ProductCatalog& productCatalog;
    CheckoutService& checkoutService;
    HeldOrderService& heldOrderService;
    ManagerApprovalService& managerApprovalService;
    ShoppingCart shoppingCart;
    optional<Product> lastScannedProduct;
    optional<ExistingOrderInfo> existingOrder;

    ScanPanel* scanPanel = nullptr;
    QDoubleSpinBox* quantitySpinBox = nullptr;
    QLineEdit* customerNameLineEdit = nullptr;
    QLabel* orderTitleLabel = nullptr;
    QTableWidget* orderTable = nullptr;
    QLabel* totalsLabel = nullptr;
    QLabel* messageLabel = nullptr;

    QWidget* createQuantityButtons(int rowIndex);
    void refreshOrderTable();
    void refreshTotals();
    void refreshOrderTitle();
    void refreshLater();
    void showMessage(const QString& messageText);
    void startNewOrder();
    void showDescription();
    void removeSelectedItem();
    void clearOrder();
    void resumeOrder();
    void editExistingOrder();
    void generateBill();
    bool orderIsSavedAsBilledOrder() const;
};
