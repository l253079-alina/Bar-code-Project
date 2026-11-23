#pragma once

#include "category.h"
#include "standardnames.h"

#include <QString>

class CategoryRepository {
public:
    virtual ~CategoryRepository() = default;
    virtual bool loadCategories(vector<Category>& categories, QString& problemDescription) = 0;
    virtual bool saveCategories(const vector<Category>& categories, QString& problemDescription) = 0;
};
