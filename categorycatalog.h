#pragma once

#include "category.h"
#include "categoryrepository.h"
#include "standardnames.h"

#include <QString>

class CategoryCatalog {
public:
    explicit CategoryCatalog(CategoryRepository& repositoryToUse);

    bool reload(QString& problemDescription);
    const vector<Category>& allCategories() const;
    optional<Category> findById(const QString& categoryId) const;
    bool addCategory(const Category& newCategory, QString& rejectionReason);
    bool updateCategory(const Category& changedCategory, QString& rejectionReason);
    bool removeCategory(const QString& categoryId, QString& rejectionReason);

private:
    CategoryRepository& categoryRepository;
    vector<Category> categories;

    static bool isValidCategoryId(const QString& categoryId);
    static bool validateDetails(const Category& category, QString& rejectionReason);
    bool acronymInUse(const QString& acronym, const QString& exceptCategoryId) const;
    bool saveChangedCategories(const vector<Category>& changedCategories, QString& problemDescription);
};
