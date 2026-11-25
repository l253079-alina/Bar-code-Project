#include "seasoncatalog.h"

SeasonCatalog::SeasonCatalog(SeasonRepository& repositoryToUse)
    : seasonRepository(repositoryToUse)
{
}

bool SeasonCatalog::reload(QString& problemDescription)
{
    return seasonRepository.loadSeasons(seasons, problemDescription);
}

const vector<Season>& SeasonCatalog::allSeasons() const
{
    return seasons;
}

bool SeasonCatalog::saveChangedSeasons(const vector<Season>& changedSeasons, QString& problemDescription)
{
    if (!seasonRepository.saveSeasons(changedSeasons, problemDescription))
        return false;
    seasons = changedSeasons;
    return true;
}

bool SeasonCatalog::addSeason(const Season& newSeason, QString& rejectionReason)
{
    if (!newSeason.startDate.isValid() || !newSeason.endDate.isValid()) {
        rejectionReason = "Please choose a valid start date and end date.";
        return false;
    }
    if (newSeason.endDate < newSeason.startDate) {
        rejectionReason = "The end date must not be before the start date.";
        return false;
    }
    if (newSeason.discountPercent <= 0.0 || newSeason.discountPercent > 100.0) {
        rejectionReason = "The discount percentage must be greater than 0 and not more than 100.";
        return false;
    }

    vector<Season> changedSeasons = seasons;
    changedSeasons.push_back(newSeason);
    return saveChangedSeasons(changedSeasons, rejectionReason);
}

bool SeasonCatalog::setSeasonActive(int seasonIndex, bool shouldBeActive, QString& rejectionReason)
{
    if (seasonIndex < 0 || seasonIndex >= static_cast<int>(seasons.size())) {
        rejectionReason = "Select a season first.";
        return false;
    }
    vector<Season> changedSeasons = seasons;
    changedSeasons[seasonIndex].isActive = shouldBeActive;
    return saveChangedSeasons(changedSeasons, rejectionReason);
}

double SeasonCatalog::activeDiscountPercentOn(const QDate& billDate) const
{
    double highestPercent = 0.0;
    for (const Season& season : seasons) {
        if (!season.isActive)
            continue;
        if (billDate < season.startDate || billDate > season.endDate)
            continue;
        highestPercent = max(highestPercent, season.discountPercent);
    }
    return highestPercent;
}
