#include "datafolderlocator.h"

#include "standardnames.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStringList>

static const QString savedFolderKey = "dataFolder";
static const QString productFileName = "Product.txt";

bool DataFolderLocator::containsProductFile(const QString& folderPath)
{
    if (folderPath.isEmpty())
        return false;
    return QDir(folderPath).exists(productFileName);
}

void DataFolderLocator::rememberDataFolder(const QString& folderPath)
{
    QSettings settings;
    settings.setValue(savedFolderKey, folderPath);
}

QString DataFolderLocator::askForDataFolder(QWidget* parentWidget)
{
    const QString chosenFolder = QFileDialog::getExistingDirectory(
        parentWidget, "Choose the folder that contains Product.txt");
    if (chosenFolder.isEmpty())
        return QString();

    if (!containsProductFile(chosenFolder)) {
        QMessageBox::warning(parentWidget, "Data folder", "That folder does not contain Product.txt.");
        return QString();
    }
    rememberDataFolder(chosenFolder);
    return chosenFolder;
}

QString DataFolderLocator::findDataFolder(QWidget* parentWidget)
{
    QSettings settings;
    QStringList candidateFolders;
    candidateFolders << settings.value(savedFolderKey).toString()
                     << QDir::currentPath()
                     << QCoreApplication::applicationDirPath();

    for (const QString& candidateFolder : candidateFolders) {
        if (containsProductFile(candidateFolder))
            return candidateFolder;
    }

    QMessageBox::information(parentWidget, "Data folder",
                             "Product.txt was not found. Please choose the folder that contains it.");
    return askForDataFolder(parentWidget);
}
