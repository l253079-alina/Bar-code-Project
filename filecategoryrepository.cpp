#include "filecategoryrepository.h"

#include "filefieldformatter.h"
#include "textfilelines.h"

static const int categoryFieldCount = 3;

FileCategoryRepository::FileCategoryRepository(const QString& categoryFilePathToUse)
    : categoryFilePath(categoryFilePathToUse)
{
}

bool FileCategoryRepository::loadCategories(vector<Category>& categories, QString& problemDescription)
{
    categories.clear();
    problemDescription.clear();

    QStringList skippedLines;
    for (const QString& line : TextFileLines::readNonEmptyLines(categoryFilePath)) {
        const QStringList fields = FileFieldFormatter::fieldsOfLine(line);
        if (fields.size() != categoryFieldCount) {
            skippedLines << line;
            continue;
        }
        Category category;
        category.categoryId = fields[0];
        category.name = FileFieldFormatter::restoreText(fields[1]);
        category.acronym = FileFieldFormatter::restoreText(fields[2]);
        categories.push_back(category);
    }

    if (!skippedLines.isEmpty())
        problemDescription = "These lines of PCategory.txt could not be read and were skipped: "
            + skippedLines.join(" | ");
    return true;
}

bool FileCategoryRepository::saveCategories(const vector<Category>& categories, QString& problemDescription)
{
    QStringList lines;
    for (const Category& category : categories) {
        QStringList fields;
        fields << category.categoryId
               << FileFieldFormatter::text(category.name)
               << FileFieldFormatter::text(category.acronym);
        lines << FileFieldFormatter::lineWithEndMarker(fields);
    }
    return TextFileLines::writeAllLines(categoryFilePath, lines, problemDescription);
}
