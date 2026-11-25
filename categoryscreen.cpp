#include "categoryscreen.h"

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

CategoryScreen::CategoryScreen(CategoryCatalog& categories, ProductCatalog& products, QWidget* parent)
    : QWidget(parent), categoryCatalog(categories), productCatalog(products)
{
    categoryTable = new QTableWidget(0, 4);
    categoryTable->setHorizontalHeaderLabels({ "Category id", "Category name", "Acronym", "Products" });
    categoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    categoryTable->setSelectionMode(QAbstractItemView::SingleSelection);
    categoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    categoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    categoryIdLineEdit = new QLineEdit;
    categoryIdLineEdit->setPlaceholderText("2 digits, first digit not zero");
    categoryIdLineEdit->setMaxLength(2);
    categoryIdLineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[1-9]\\d{0,1}"), this));
    categoryNameLineEdit = new QLineEdit;
    categoryNameLineEdit->setPlaceholderText("For example Beverages");
    categoryAcronymLineEdit = new QLineEdit;
    categoryAcronymLineEdit->setPlaceholderText("For example BEV");

    QPushButton* addButton = new QPushButton("Add category");
    QPushButton* updateButton = new QPushButton("Update selected");
    QPushButton* removeButton = new QPushButton("Remove selected");
    QPushButton* clearButton = new QPushButton("Clear form");

    QHBoxLayout* formLayout = new QHBoxLayout;
    formLayout->addWidget(new QLabel("Id"));
    formLayout->addWidget(categoryIdLineEdit, 1);
    formLayout->addWidget(new QLabel("Name"));
    formLayout->addWidget(categoryNameLineEdit, 2);
    formLayout->addWidget(new QLabel("Acronym"));
    formLayout->addWidget(categoryAcronymLineEdit, 1);

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(addButton);
    buttonsLayout->addWidget(updateButton);
    buttonsLayout->addWidget(removeButton);
    buttonsLayout->addWidget(clearButton);

    messageLabel = new QLabel("The first two digits of every barcode are the category id.");
    messageLabel->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Product categories (stored in PCategory.txt)"));
    layout->addWidget(categoryTable, 1);
    layout->addLayout(formLayout);
    layout->addLayout(buttonsLayout);
    layout->addWidget(messageLabel);

    connect(categoryTable, &QTableWidget::itemSelectionChanged, this, [this] { showSelectedCategoryInForm(); });
    connect(addButton, &QPushButton::clicked, this, [this] { addCategory(); });
    connect(updateButton, &QPushButton::clicked, this, [this] { updateSelectedCategory(); });
    connect(removeButton, &QPushButton::clicked, this, [this] { removeSelectedCategory(); });
    connect(clearButton, &QPushButton::clicked, this, [this] { clearForm(); });

    refresh();
}

void CategoryScreen::refresh()
{
    const vector<Category>& categories = categoryCatalog.allCategories();
    categoryTable->setRowCount(static_cast<int>(categories.size()));
    for (size_t categoryIndex = 0; categoryIndex < categories.size(); ++categoryIndex) {
        const Category& category = categories[categoryIndex];
        int productCountInCategory = 0;
        for (const Product& product : productCatalog.allProducts()) {
            if (product.categoryId == category.categoryId)
                ++productCountInCategory;
        }

        QStringList cellTexts;
        cellTexts << category.categoryId << category.name << category.acronym
                  << QString::number(productCountInCategory);
        for (int columnIndex = 0; columnIndex < cellTexts.size(); ++columnIndex)
            categoryTable->setItem(static_cast<int>(categoryIndex), columnIndex,
                                   new QTableWidgetItem(cellTexts[columnIndex]));
    }
}

QString CategoryScreen::selectedCategoryId() const
{
    const int selectedRow = categoryTable->currentRow();
    if (selectedRow < 0 || selectedRow >= categoryTable->rowCount())
        return QString();
    return categoryTable->item(selectedRow, 0)->text();
}

void CategoryScreen::showSelectedCategoryInForm()
{
    const int selectedRow = categoryTable->currentRow();
    if (selectedRow < 0 || selectedRow >= categoryTable->rowCount())
        return;
    categoryIdLineEdit->setText(categoryTable->item(selectedRow, 0)->text());
    categoryNameLineEdit->setText(categoryTable->item(selectedRow, 1)->text());
    categoryAcronymLineEdit->setText(categoryTable->item(selectedRow, 2)->text());
}

void CategoryScreen::clearForm()
{
    categoryTable->clearSelection();
    categoryIdLineEdit->clear();
    categoryNameLineEdit->clear();
    categoryAcronymLineEdit->clear();
}

void CategoryScreen::addCategory()
{
    Category newCategory;
    newCategory.categoryId = categoryIdLineEdit->text();
    newCategory.name = categoryNameLineEdit->text();
    newCategory.acronym = categoryAcronymLineEdit->text();

    QString rejectionReason;
    if (!categoryCatalog.addCategory(newCategory, rejectionReason)) {
        QMessageBox::warning(this, "Add category", rejectionReason);
        return;
    }
    refresh();
    clearForm();
    messageLabel->setText("Category " + newCategory.categoryId + " was added.");
}

void CategoryScreen::updateSelectedCategory()
{
    const QString chosenCategoryId = selectedCategoryId();
    if (chosenCategoryId.isEmpty()) {
        messageLabel->setText("Select a category in the table to update it.");
        return;
    }

    Category changedCategory;
    changedCategory.categoryId = chosenCategoryId;
    changedCategory.name = categoryNameLineEdit->text();
    changedCategory.acronym = categoryAcronymLineEdit->text();

    QString rejectionReason;
    if (!categoryCatalog.updateCategory(changedCategory, rejectionReason)) {
        QMessageBox::warning(this, "Update category", rejectionReason);
        return;
    }
    refresh();
    messageLabel->setText("Category " + chosenCategoryId + " was updated. The category id cannot be changed.");
}

void CategoryScreen::removeSelectedCategory()
{
    const QString chosenCategoryId = selectedCategoryId();
    if (chosenCategoryId.isEmpty()) {
        messageLabel->setText("Select a category in the table to remove it.");
        return;
    }
    if (productCatalog.categoryInUse(chosenCategoryId)) {
        QMessageBox::warning(this, "Remove category",
                             "Category " + chosenCategoryId + " is used by products and cannot be removed.");
        return;
    }

    QString rejectionReason;
    if (!categoryCatalog.removeCategory(chosenCategoryId, rejectionReason)) {
        QMessageBox::warning(this, "Remove category", rejectionReason);
        return;
    }
    refresh();
    clearForm();
    messageLabel->setText("Category " + chosenCategoryId + " was removed.");
}
