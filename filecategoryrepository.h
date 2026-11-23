#pragma once

#include "categoryrepository.h"

class FileCategoryRepository : public CategoryRepository {
public:
    explicit FileCategoryRepository(const QString& categoryFilePathToUse);
    bool loadCategories(vector<Category>& categories, QString& problemDescription) override;
    bool saveCategories(const vector<Category>& categories, QString& problemDescription) override;

private:
    QString categoryFilePath;
};
