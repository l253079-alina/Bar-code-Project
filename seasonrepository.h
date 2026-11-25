#pragma once

#include "season.h"
#include "standardnames.h"

#include <QString>

class SeasonRepository {
public:
    virtual ~SeasonRepository() = default;
    virtual bool loadSeasons(vector<Season>& seasons, QString& problemDescription) = 0;
    virtual bool saveSeasons(const vector<Season>& seasons, QString& problemDescription) = 0;
};
