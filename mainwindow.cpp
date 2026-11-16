#include "mainwindow.h"
#include "barcodegeneratorscreen.h"
#include "checkoutservice.h"
#include "datafolderlocator.h"
#include "filebillrepository.h"
#include "fileorderrepository.h"
#include "fileproductrepository.h"
#include "orderscreen.h"
#include "productcatalog.h"
#include "productinfoscreen.h"

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

MainWindow::MainWindow(const QString& dataFolderToUse): dataFolderPath(dataFolderToUse){
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
    orderRepository = make_unique<FileOrderRepository>(dataFolder.filePath("Order.txt"),
                                                       dataFolder.filePath("OrderDetail.txt"));
    billRepository = make_unique<FileBillRepository>(dataFolder.filePath("Bill.txt"));
    checkoutService = make_unique<CheckoutService>(*orderRepository, *billRepository);

    QString problemDescription;
    const bool productsLoaded = productCatalog->reload(problemDescription);
    if (!productsLoaded)
        QMessageBox::critical(this, "Products", problemDescription);
    else if (!problemDescription.isEmpty())
        QMessageBox::warning(this, "Products", problemDescription);
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

    orderScreen = new OrderScreen(*productCatalog, *checkoutService);
    productInfoScreen = new ProductInfoScreen(*productCatalog);
    barcodeGeneratorScreen = new BarcodeGeneratorScreen(*productCatalog, dataFolderPath);

    tabWidget = new QTabWidget;
    orderTabIndex = tabWidget->addTab(orderScreen, "Record order");
    productInfoTabIndex = tabWidget->addTab(productInfoScreen, "Product description");
    barcodeGeneratorTabIndex = tabWidget->addTab(barcodeGeneratorScreen, "Barcode generator");

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(topBarLayout);
    layout->addWidget(tabWidget, 1);
    QWidget* centralWidget = new QWidget;
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    connect(roleComboBox, &QComboBox::currentTextChanged, this, [this] { applyRole(); });
    connect(tabWidget, &QTabWidget::currentChanged, this, [this] { stopAllScanning(); });
    connect(dataFolderButton, &QPushButton::clicked, this, [this] { changeDataFolder(); });
}

void MainWindow::applyRole(){
    stopAllScanning();
    const bool roleIsCashier = roleComboBox->currentText() == cashierRoleName;

    tabWidget->setTabVisible(orderTabIndex, roleIsCashier);
    tabWidget->setTabVisible(productInfoTabIndex, true);
    tabWidget->setTabVisible(barcodeGeneratorTabIndex, !roleIsCashier);

    if (roleIsCashier)
        tabWidget->setCurrentIndex(orderTabIndex);
    else
        tabWidget->setCurrentIndex(productInfoTabIndex);
}

void MainWindow::stopAllScanning(){
    orderScreen->stopScanning();
    productInfoScreen->stopScanning();
}

void MainWindow::changeDataFolder(){
    const QString chosenFolder = DataFolderLocator::askForDataFolder(this);
    if (chosenFolder.isEmpty())
        return;
    QMessageBox::information(this, "Data folder","The new data folder will be used the next time you start the application.");
}
