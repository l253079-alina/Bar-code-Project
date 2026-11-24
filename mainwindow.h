#pragma once

#include "standardnames.h"

#include <QMainWindow>
#include <QString>

class BarcodeGeneratorScreen;
class BillsScreen;
class CategoryCatalog;
class CategoryScreen;
class CheckoutService;
class FileBillRepository;
class FileCategoryRepository;
class FileOrderRepository;
class FileProductRepository;
class FileSeasonRepository;
class HeldOrderService;
class ManagerApprovalService;
class OrderScreen;
class ProductCatalog;
class ProductInfoScreen;
class ProductManagementScreen;
class QComboBox;
class QTabWidget;
class ReportService;
class ReportsScreen;
class SeasonCatalog;
class SeasonScreen;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(const QString& dataFolderToUse);
    ~MainWindow() override;

private:
    QString dataFolderPath;
    unique_ptr<FileProductRepository> productRepository;
    unique_ptr<ProductCatalog> productCatalog;
    unique_ptr<FileCategoryRepository> categoryRepository;
    unique_ptr<CategoryCatalog> categoryCatalog;
    unique_ptr<FileSeasonRepository> seasonRepository;
    unique_ptr<SeasonCatalog> seasonCatalog;
    unique_ptr<FileOrderRepository> orderRepository;
    unique_ptr<FileBillRepository> billRepository;
    unique_ptr<CheckoutService> checkoutService;
    unique_ptr<HeldOrderService> heldOrderService;
    unique_ptr<ManagerApprovalService> managerApprovalService;
    unique_ptr<ReportService> reportService;

    QComboBox* roleComboBox = nullptr;
    QTabWidget* tabWidget = nullptr;
    OrderScreen* orderScreen = nullptr;
    ProductInfoScreen* productInfoScreen = nullptr;
    BillsScreen* billsScreen = nullptr;
    CategoryScreen* categoryScreen = nullptr;
    ProductManagementScreen* productManagementScreen = nullptr;
    BarcodeGeneratorScreen* barcodeGeneratorScreen = nullptr;
    SeasonScreen* seasonScreen = nullptr;
    ReportsScreen* reportsScreen = nullptr;

    int orderTabIndex = -1;
    int productInfoTabIndex = -1;
    int billsTabIndex = -1;
    int categoryTabIndex = -1;
    int productManagementTabIndex = -1;
    int barcodeGeneratorTabIndex = -1;
    int seasonTabIndex = -1;
    int reportsTabIndex = -1;

    void createServices();
    void createScreens();
    void applyRole();
    void stopAllScanning();
    void refreshScreenAt(int tabIndex);
    void changeDataFolder();
};
