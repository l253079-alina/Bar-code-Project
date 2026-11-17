#pragma once

#include <QString>

class QWidget;

class DataFolderLocator {
public:
    static QString findDataFolder(QWidget* parentWidget);
    static QString askForDataFolder(QWidget* parentWidget);
    static void rememberDataFolder(const QString& folderPath);
    static bool containsProductFile(const QString& folderPath);
};
