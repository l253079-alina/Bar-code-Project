#pragma once

#include "categorycatalog.h"
#include "productcatalog.h"

#include <QWidget>

class QLabel;
class QLineEdit;
class QTableWidget;

class CategoryScreen : public QWidget {
public:
    CategoryScreen(CategoryCatalog& categories, ProductCatalog& products, QWidget* parent = nullptr);
    void refresh();

private:
    CategoryCatalog& categoryCatalog;
    ProductCatalog& productCatalog;

    QTableWidget* categoryTable = nullptr;
    QLineEdit* categoryIdLineEdit = nullptr;
    QLineEdit* categoryNameLineEdit = nullptr;
    QLineEdit* categoryAcronymLineEdit = nullptr;
    QLabel* messageLabel = nullptr;

    void showSelectedCategoryInForm();
    void addCategory();
    void updateSelectedCategory();
    void removeSelectedCategory();
    void clearForm();
    QString selectedCategoryId() const;
};
