#pragma once

#include "standardnames.h"

#include <QMainWindow>
#include <QString>

class BarcodeGeneratorScreen;
class CheckoutService;
class FileBillRepository;
class FileOrderRepository;
class FileProductRepository;
class OrderScreen;
class ProductCatalog;
class ProductInfoScreen;
class QComboBox;
class QTabWidget;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(const QString& dataFolderToUse);
    ~MainWindow() override;

private:
    QString dataFolderPath;
    unique_ptr<FileProductRepository> productRepository;
    unique_ptr<ProductCatalog> productCatalog;
    unique_ptr<FileOrderRepository> orderRepository;
    unique_ptr<FileBillRepository> billRepository;
    unique_ptr<CheckoutService> checkoutService;

    QComboBox* roleComboBox = nullptr;
    QTabWidget* tabWidget = nullptr;
    OrderScreen* orderScreen = nullptr;
    ProductInfoScreen* productInfoScreen = nullptr;
    BarcodeGeneratorScreen* barcodeGeneratorScreen = nullptr;
    int orderTabIndex = -1;
    int productInfoTabIndex = -1;
    int barcodeGeneratorTabIndex = -1;

    void createServices();
    void createScreens();
    void applyRole();
    void stopAllScanning();
    void changeDataFolder();
};
