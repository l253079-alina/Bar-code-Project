#include "fileseasonrepository.h"

#include "filefieldformatter.h"
#include "orderconstants.h"
#include "textfilelines.h"

static const int seasonFieldCount = 4;
static const QString activeText = "true";
static const QString inactiveText = "false";

FileSeasonRepository::FileSeasonRepository(const QString& seasonFilePathToUse)
    : seasonFilePath(seasonFilePathToUse)
{
}

bool FileSeasonRepository::loadSeasons(vector<Season>& seasons, QString& problemDescription)
{
    seasons.clear();
    problemDescription.clear();

    QStringList skippedLines;
    for (const QString& line : TextFileLines::readNonEmptyLines(seasonFilePath)) {
        const QStringList fields = FileFieldFormatter::fieldsOfLine(line);
        if (fields.size() != seasonFieldCount) {
            skippedLines << line;
            continue;
        }

        bool percentIsNumber = false;
        const double discountPercent = fields[2].toDouble(&percentIsNumber);
        const QString activeField = fields[3].toLower();
        Season season;
        season.startDate = QDate::fromString(fields[0], OrderConstants::dateFormat);
        season.endDate = QDate::fromString(fields[1], OrderConstants::dateFormat);
        season.discountPercent = discountPercent;
        season.isActive = activeField == activeText;

        const bool activeFieldIsValid = activeField == activeText || activeField == inactiveText;
        if (!season.startDate.isValid() || !season.endDate.isValid() || !percentIsNumber || !activeFieldIsValid) {
            skippedLines << line;
            continue;
        }
        seasons.push_back(season);
    }

    if (!skippedLines.isEmpty())
        problemDescription = "These lines of Seasons.txt could not be read and were skipped: "
            + skippedLines.join(" | ");
    return true;
}

bool FileSeasonRepository::saveSeasons(const vector<Season>& seasons, QString& problemDescription)
{
    QStringList lines;
    for (const Season& season : seasons) {
        QString activeField = inactiveText;
        if (season.isActive)
            activeField = activeText;

        QStringList fields;
        fields << season.startDate.toString(OrderConstants::dateFormat)
               << season.endDate.toString(OrderConstants::dateFormat)
               << FileFieldFormatter::quantity(season.discountPercent)
               << activeField;
        lines << FileFieldFormatter::lineWithEndMarker(fields);
    }
    return TextFileLines::writeAllLines(seasonFilePath, lines, problemDescription);
}
