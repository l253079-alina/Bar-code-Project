#include "seasonscreen.h"

#include "filefieldformatter.h"
#include "orderconstants.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

SeasonScreen::SeasonScreen(SeasonCatalog& catalog, QWidget* parent)
    : QWidget(parent), seasonCatalog(catalog)
{
    seasonTable = new QTableWidget(0, 4);
    seasonTable->setHorizontalHeaderLabels({ "Start date", "End date", "Discount (%)", "Active" });
    seasonTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    seasonTable->setSelectionMode(QAbstractItemView::SingleSelection);
    seasonTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    seasonTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    startDateEdit = new QDateEdit(QDate::currentDate());
    startDateEdit->setCalendarPopup(true);
    startDateEdit->setDisplayFormat(OrderConstants::dateFormat);
    endDateEdit = new QDateEdit(QDate::currentDate().addDays(30));
    endDateEdit->setCalendarPopup(true);
    endDateEdit->setDisplayFormat(OrderConstants::dateFormat);

    discountPercentSpinBox = new QDoubleSpinBox;
    discountPercentSpinBox->setDecimals(2);
    discountPercentSpinBox->setRange(0.01, 100.0);
    discountPercentSpinBox->setValue(10.0);
    discountPercentSpinBox->setSuffix(" %");

    activeCheckBox = new QCheckBox("Active");
    activeCheckBox->setChecked(true);

    QPushButton* addButton = new QPushButton("Add season");
    QPushButton* toggleButton = new QPushButton("Activate / deactivate selected");

    QHBoxLayout* formLayout = new QHBoxLayout;
    formLayout->addWidget(new QLabel("Start"));
    formLayout->addWidget(startDateEdit);
    formLayout->addWidget(new QLabel("End"));
    formLayout->addWidget(endDateEdit);
    formLayout->addWidget(new QLabel("Discount"));
    formLayout->addWidget(discountPercentSpinBox);
    formLayout->addWidget(activeCheckBox);
    formLayout->addWidget(addButton);

    messageLabel = new QLabel("A bill made inside an active season gets the seasonal discount automatically.");
    messageLabel->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Seasons for special discounts (stored in Seasons.txt)"));
    layout->addWidget(seasonTable, 1);
    layout->addWidget(toggleButton);
    layout->addLayout(formLayout);
    layout->addWidget(messageLabel);

    connect(addButton, &QPushButton::clicked, this, [this] { addSeason(); });
    connect(toggleButton, &QPushButton::clicked, this, [this] { toggleSelectedSeason(); });

    refresh();
}

void SeasonScreen::refresh()
{
    const vector<Season>& seasons = seasonCatalog.allSeasons();
    seasonTable->setRowCount(static_cast<int>(seasons.size()));
    for (size_t seasonIndex = 0; seasonIndex < seasons.size(); ++seasonIndex) {
        const Season& season = seasons[seasonIndex];
        QString activeText = "No";
        if (season.isActive)
            activeText = "Yes";

        QStringList cellTexts;
        cellTexts << season.startDate.toString(OrderConstants::dateFormat)
                  << season.endDate.toString(OrderConstants::dateFormat)
                  << FileFieldFormatter::quantity(season.discountPercent) << activeText;
        for (int columnIndex = 0; columnIndex < cellTexts.size(); ++columnIndex)
            seasonTable->setItem(static_cast<int>(seasonIndex), columnIndex, new QTableWidgetItem(cellTexts[columnIndex]));
    }
}

void SeasonScreen::addSeason()
{
    Season newSeason;
    newSeason.startDate = startDateEdit->date();
    newSeason.endDate = endDateEdit->date();
    newSeason.discountPercent = discountPercentSpinBox->value();
    newSeason.isActive = activeCheckBox->isChecked();

    QString rejectionReason;
    if (!seasonCatalog.addSeason(newSeason, rejectionReason)) {
        QMessageBox::warning(this, "Add season", rejectionReason);
        return;
    }
    refresh();
    messageLabel->setText("The season was added.");
}

void SeasonScreen::toggleSelectedSeason()
{
    const int selectedRow = seasonTable->currentRow();
    const vector<Season>& seasons = seasonCatalog.allSeasons();
    if (selectedRow < 0 || selectedRow >= static_cast<int>(seasons.size())) {
        messageLabel->setText("Select a season in the table first.");
        return;
    }

    const bool newActiveState = !seasons[selectedRow].isActive;
    QString rejectionReason;
    if (!seasonCatalog.setSeasonActive(selectedRow, newActiveState, rejectionReason)) {
        QMessageBox::warning(this, "Seasons", rejectionReason);
        return;
    }
    refresh();
    seasonTable->setCurrentCell(selectedRow, 0);
    if (newActiveState)
        messageLabel->setText("The season is now active.");
    else
        messageLabel->setText("The season is now inactive.");
}
