#include "mainwindow.h"

#include "barcodegeneratorscreen.h"
#include "billsscreen.h"
#include "categorycatalog.h"
#include "categoryscreen.h"
#include "checkoutservice.h"
#include "datafolderlocator.h"
#include "filebillrepository.h"
#include "filecategoryrepository.h"
#include "fileorderrepository.h"
#include "fileproductrepository.h"
#include "fileseasonrepository.h"
#include "heldorderservice.h"
#include "managerapprovalservice.h"
#include "orderscreen.h"
#include "productcatalog.h"
#include "productinfoscreen.h"
#include "productmanagementscreen.h"
#include "reportservice.h"
#include "reportsscreen.h"
#include "seasoncatalog.h"
#include "seasonscreen.h"

#include <QComboBox>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

static const QString cashierRoleName = "Cashier";
static const QString managerRoleName = "Manager";

MainWindow::MainWindow(const QString& dataFolderToUse)
    : dataFolderPath(dataFolderToUse)
{
    setWindowTitle("BarcodeSix point of sale");
    resize(1200, 720);

    createServices();
    createScreens();
    applyRole();
}

MainWindow::~MainWindow() = default;

void MainWindow::createServices(){
    const QDir dataFolder(dataFolderPath);
    productRepository = make_unique<FileProductRepository>(dataFolder.filePath("Product.txt"));
    productCatalog = make_unique<ProductCatalog>(*productRepository);
    categoryRepository = make_unique<FileCategoryRepository>(dataFolder.filePath("PCategory.txt"));
    categoryCatalog = make_unique<CategoryCatalog>(*categoryRepository);
    seasonRepository = make_unique<FileSeasonRepository>(dataFolder.filePath("Seasons.txt"));
    seasonCatalog = make_unique<SeasonCatalog>(*seasonRepository);
    orderRepository = make_unique<FileOrderRepository>(dataFolder.filePath("Order.txt"),
                                                       dataFolder.filePath("OrderDetail.txt"));
    billRepository = make_unique<FileBillRepository>(dataFolder.filePath("Bill.txt"),
                                                     dataFolder.filePath("BillDetail.txt"));
    checkoutService = make_unique<CheckoutService>(*orderRepository, *billRepository, *productCatalog, *seasonCatalog);
    heldOrderService = make_unique<HeldOrderService>(*orderRepository, *orderRepository, *productCatalog, *billRepository);
    managerApprovalService = make_unique<ManagerApprovalService>(dataFolderPath);
    reportService = make_unique<ReportService>(*orderRepository, *billRepository, *productCatalog);

    QString problemDescription;
    const bool productsLoaded = productCatalog->reload(problemDescription);
    if (!productsLoaded)
        QMessageBox::critical(this, "Products", problemDescription);
    else if (!problemDescription.isEmpty())
        QMessageBox::warning(this, "Products", problemDescription);

    if (categoryCatalog->reload(problemDescription) && !problemDescription.isEmpty())
        QMessageBox::warning(this, "Categories", problemDescription);
    if (seasonCatalog->reload(problemDescription) && !problemDescription.isEmpty())
        QMessageBox::warning(this, "Seasons", problemDescription);
}

void MainWindow::createScreens()
{
    roleComboBox = new QComboBox;
    roleComboBox->addItems({ cashierRoleName, managerRoleName });

    QPushButton* dataFolderButton = new QPushButton("Change data folder...");
    QLabel* dataFolderLabel = new QLabel("Data folder: " + QDir::toNativeSeparators(dataFolderPath));

    QHBoxLayout* topBarLayout = new QHBoxLayout;
    topBarLayout->addWidget(new QLabel("Role:"));
    topBarLayout->addWidget(roleComboBox);
    topBarLayout->addSpacing(20);
    topBarLayout->addWidget(dataFolderLabel, 1);
    topBarLayout->addWidget(dataFolderButton);

    orderScreen = new OrderScreen(*productCatalog, *checkoutService, *heldOrderService, *managerApprovalService);
    productInfoScreen = new ProductInfoScreen(*productCatalog);
    billsScreen = new BillsScreen(*billRepository, *productCatalog);
    categoryScreen = new CategoryScreen(*categoryCatalog, *productCatalog);
    productManagementScreen = new ProductManagementScreen(*productCatalog, *categoryCatalog, dataFolderPath);
    barcodeGeneratorScreen = new BarcodeGeneratorScreen(*productCatalog, *categoryCatalog, dataFolderPath);
    seasonScreen = new SeasonScreen(*seasonCatalog);
    reportsScreen = new ReportsScreen(*reportService);

    tabWidget = new QTabWidget;
    orderTabIndex = tabWidget->addTab(orderScreen, "Record order");
    productInfoTabIndex = tabWidget->addTab(productInfoScreen, "Product description");
    billsTabIndex = tabWidget->addTab(billsScreen, "Bills");
    categoryTabIndex = tabWidget->addTab(categoryScreen, "Categories");
    productManagementTabIndex = tabWidget->addTab(productManagementScreen, "Products and inventory");
    barcodeGeneratorTabIndex = tabWidget->addTab(barcodeGeneratorScreen, "Barcode generator");
    seasonTabIndex = tabWidget->addTab(seasonScreen, "Seasonal discounts");
    reportsTabIndex = tabWidget->addTab(reportsScreen, "Reports");

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(topBarLayout);
    layout->addWidget(tabWidget, 1);
    QWidget* centralWidget = new QWidget;
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    connect(roleComboBox, &QComboBox::currentTextChanged, this, [this] { applyRole(); });
    connect(tabWidget, &QTabWidget::currentChanged, this, [this](int tabIndex) {
        stopAllScanning();
        refreshScreenAt(tabIndex);
    });
    connect(dataFolderButton, &QPushButton::clicked, this, [this] { changeDataFolder(); });
}

void MainWindow::applyRole(){
    stopAllScanning();
    const bool roleIsCashier = roleComboBox->currentText() == cashierRoleName;
    const bool roleIsManager = !roleIsCashier;

    tabWidget->setTabVisible(orderTabIndex, roleIsCashier);
    tabWidget->setTabVisible(productInfoTabIndex, true);
    tabWidget->setTabVisible(billsTabIndex, true);
    tabWidget->setTabVisible(categoryTabIndex, roleIsManager);
    tabWidget->setTabVisible(productManagementTabIndex, roleIsManager);
    tabWidget->setTabVisible(barcodeGeneratorTabIndex, roleIsManager);
    tabWidget->setTabVisible(seasonTabIndex, roleIsManager);
    tabWidget->setTabVisible(reportsTabIndex, roleIsManager);

    if (roleIsCashier)
        tabWidget->setCurrentIndex(orderTabIndex);
    else
        tabWidget->setCurrentIndex(productManagementTabIndex);
}

void MainWindow::stopAllScanning(){
    orderScreen->stopScanning();
    productInfoScreen->stopScanning();
}

void MainWindow::refreshScreenAt(int tabIndex){
    if (tabIndex == billsTabIndex)
        billsScreen->refresh();
    else if (tabIndex == categoryTabIndex)
        categoryScreen->refresh();
    else if (tabIndex == productManagementTabIndex)
        productManagementScreen->refresh();
    else if (tabIndex == barcodeGeneratorTabIndex)
        barcodeGeneratorScreen->reloadData();
    else if (tabIndex == seasonTabIndex)
        seasonScreen->refresh();
}

void MainWindow::changeDataFolder(){
    const QString chosenFolder = DataFolderLocator::askForDataFolder(this);
    if (chosenFolder.isEmpty())
        return;
    QMessageBox::information(this, "Data folder","The new data folder will be used the next time you start the application.");
}
