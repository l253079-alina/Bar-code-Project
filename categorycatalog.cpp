#include "categorycatalog.h"

static const int categoryIdLength = 2;

CategoryCatalog::CategoryCatalog(CategoryRepository& repositoryToUse)
    : categoryRepository(repositoryToUse)
{
}

bool CategoryCatalog::reload(QString& problemDescription)
{
    return categoryRepository.loadCategories(categories, problemDescription);
}

const vector<Category>& CategoryCatalog::allCategories() const
{
    return categories;
}

optional<Category> CategoryCatalog::findById(const QString& categoryId) const
{
    for (const Category& category : categories) {
        if (category.categoryId == categoryId)
            return category;
    }
    return nullopt;
}

bool CategoryCatalog::isValidCategoryId(const QString& categoryId)
{
    if (categoryId.size() != categoryIdLength)
        return false;
    for (const QChar character : categoryId) {
        if (!character.isDigit())
            return false;
    }
    return categoryId[0] != QChar('0');
}

bool CategoryCatalog::validateDetails(const Category& category, QString& rejectionReason)
{
    if (!isValidCategoryId(category.categoryId)) {
        rejectionReason = "The category id must be exactly 2 digits and the first digit must not be zero.";
        return false;
    }
    if (category.name.trimmed().isEmpty()) {
        rejectionReason = "Please enter the category name.";
        return false;
    }
    if (category.acronym.trimmed().isEmpty()) {
        rejectionReason = "Please enter the category acronym.";
        return false;
    }
    if (category.acronym.contains(QChar(' ')) || category.acronym.contains(QChar('#'))) {
        rejectionReason = "The category acronym must not contain spaces or the # character.";
        return false;
    }
    if (category.name.contains(QChar('#'))) {
        rejectionReason = "The category name must not contain the # character.";
        return false;
    }
    return true;
}

bool CategoryCatalog::acronymInUse(const QString& acronym, const QString& exceptCategoryId) const
{
    for (const Category& category : categories) {
        if (category.categoryId == exceptCategoryId)
            continue;
        if (category.acronym.compare(acronym, Qt::CaseInsensitive) == 0)
            return true;
    }
    return false;
}

bool CategoryCatalog::saveChangedCategories(const vector<Category>& changedCategories, QString& problemDescription)
{
    if (!categoryRepository.saveCategories(changedCategories, problemDescription))
        return false;
    categories = changedCategories;
    return true;
}

bool CategoryCatalog::addCategory(const Category& newCategory, QString& rejectionReason)
{
    Category cleanedCategory = newCategory;
    cleanedCategory.categoryId = cleanedCategory.categoryId.trimmed();
    cleanedCategory.name = cleanedCategory.name.simplified();
    cleanedCategory.acronym = cleanedCategory.acronym.trimmed();

    if (!validateDetails(cleanedCategory, rejectionReason))
        return false;
    if (findById(cleanedCategory.categoryId).has_value()) {
        rejectionReason = "A category with id " + cleanedCategory.categoryId + " already exists.";
        return false;
    }
    if (acronymInUse(cleanedCategory.acronym, QString())) {
        rejectionReason = "The acronym " + cleanedCategory.acronym + " is already used by another category.";
        return false;
    }

    vector<Category> changedCategories = categories;
    changedCategories.push_back(cleanedCategory);
    return saveChangedCategories(changedCategories, rejectionReason);
}

bool CategoryCatalog::updateCategory(const Category& changedCategory, QString& rejectionReason)
{
    Category cleanedCategory = changedCategory;
    cleanedCategory.name = cleanedCategory.name.simplified();
    cleanedCategory.acronym = cleanedCategory.acronym.trimmed();

    if (!validateDetails(cleanedCategory, rejectionReason))
        return false;
    if (acronymInUse(cleanedCategory.acronym, cleanedCategory.categoryId)) {
        rejectionReason = "The acronym " + cleanedCategory.acronym + " is already used by another category.";
        return false;
    }

    vector<Category> changedCategories = categories;
    bool categoryWasFound = false;
    for (Category& category : changedCategories) {
        if (category.categoryId == cleanedCategory.categoryId) {
            category = cleanedCategory;
            categoryWasFound = true;
        }
    }
    if (!categoryWasFound) {
        rejectionReason = "Category " + cleanedCategory.categoryId + " was not found.";
        return false;
    }
    return saveChangedCategories(changedCategories, rejectionReason);
}

bool CategoryCatalog::removeCategory(const QString& categoryId, QString& rejectionReason)
{
    vector<Category> changedCategories;
    bool categoryWasFound = false;
    for (const Category& category : categories) {
        if (category.categoryId == categoryId)
            categoryWasFound = true;
        else
            changedCategories.push_back(category);
    }
    if (!categoryWasFound) {
        rejectionReason = "Category " + categoryId + " was not found.";
        return false;
    }
    return saveChangedCategories(changedCategories, rejectionReason);
}
