#pragma once

#include "season.h"
#include "seasonrepository.h"
#include "standardnames.h"

#include <QDate>
#include <QString>

class SeasonCatalog {
public:
    explicit SeasonCatalog(SeasonRepository& repositoryToUse);

    bool reload(QString& problemDescription);
    const vector<Season>& allSeasons() const;
    bool addSeason(const Season& newSeason, QString& rejectionReason);
    bool setSeasonActive(int seasonIndex, bool shouldBeActive, QString& rejectionReason);
    double activeDiscountPercentOn(const QDate& billDate) const;

private:
    SeasonRepository& seasonRepository;
    vector<Season> seasons;

    bool saveChangedSeasons(const vector<Season>& changedSeasons, QString& problemDescription);
};
