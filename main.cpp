#include "datafolderlocator.h"
#include "mainwindow.h"

#include <QApplication>

int main(int argumentCount, char** argumentValues)
{
    QApplication application(argumentCount, argumentValues);
    QCoreApplication::setOrganizationName("BarcodeSix");
    QCoreApplication::setApplicationName("PointOfSale");

    const QString dataFolderPath = DataFolderLocator::findDataFolder(nullptr);
    if (dataFolderPath.isEmpty())
        return 0;

    MainWindow mainWindow(dataFolderPath);
    mainWindow.show();
    return application.exec();
}
