#pragma once

#include "seasonrepository.h"

class FileSeasonRepository : public SeasonRepository {
public:
    explicit FileSeasonRepository(const QString& seasonFilePathToUse);
    bool loadSeasons(vector<Season>& seasons, QString& problemDescription) override;
    bool saveSeasons(const vector<Season>& seasons, QString& problemDescription) override;

private:
    QString seasonFilePath;
};
