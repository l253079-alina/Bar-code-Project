#include "reportsscreen.h"

#include "orderconstants.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDateEdit>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

static const int incompleteOrdersIndex = 0;
static const int completedOrdersIndex = 1;
static const int cashSalesIndex = 2;
static const int allSalesIndex = 3;
static const int topSoldIndex = 4;
static const int topRemainingIndex = 5;

static const int todayPeriodIndex = 0;
static const int weekPeriodIndex = 1;
static const int monthPeriodIndex = 2;
static const int yearPeriodIndex = 3;
static const int specifiedPeriodIndex = 4;

ReportsScreen::ReportsScreen(ReportService& service, QWidget* parent)
    : QWidget(parent), reportService(service)
{
    reportComboBox = new QComboBox;
    reportComboBox->addItems({ "Incomplete orders (not paid or archived)", "Completed orders",
                               "Cash sales", "All sales with discounts",
                               "Top ten most sold items", "Top ten items with most remaining stock" });

    periodComboBox = new QComboBox;
    periodComboBox->addItems({ "Daily (today)", "Weekly (this week)", "Monthly (this month)", "Yearly (this year)",
                               "Specified period" });

    fromDateEdit = new QDateEdit(QDate::currentDate());
    fromDateEdit->setCalendarPopup(true);
    fromDateEdit->setDisplayFormat(OrderConstants::dateFormat);
    toDateEdit = new QDateEdit(QDate::currentDate());
    toDateEdit->setCalendarPopup(true);
    toDateEdit->setDisplayFormat(OrderConstants::dateFormat);

    QPushButton* generateButton = new QPushButton("Generate report");

    reportTable = new QTableWidget(0, 0);
    reportTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    reportTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    reportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    summaryLabel = new QLabel("Choose a report and press Generate report.");
    summaryLabel->setWordWrap(true);
    summaryLabel->setStyleSheet("font-weight: bold;");

    QHBoxLayout* reportLayout = new QHBoxLayout;
    reportLayout->addWidget(new QLabel("Report"));
    reportLayout->addWidget(reportComboBox, 1);
    reportLayout->addWidget(generateButton);

    QHBoxLayout* periodLayout = new QHBoxLayout;
    periodLayout->addWidget(new QLabel("Period"));
    periodLayout->addWidget(periodComboBox);
    periodLayout->addWidget(new QLabel("From"));
    periodLayout->addWidget(fromDateEdit);
    periodLayout->addWidget(new QLabel("To"));
    periodLayout->addWidget(toDateEdit);
    periodLayout->addStretch(1);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(reportLayout);
    layout->addLayout(periodLayout);
    layout->addWidget(reportTable, 1);
    layout->addWidget(summaryLabel);

    connect(reportComboBox, &QComboBox::currentIndexChanged, this, [this] { updatePeriodControls(); });
    connect(periodComboBox, &QComboBox::currentIndexChanged, this, [this] { applyPeriodChoice(); });
    connect(generateButton, &QPushButton::clicked, this, [this] { generateReport(); });

    applyPeriodChoice();
    updatePeriodControls();
}

void ReportsScreen::updatePeriodControls()
{
    const int chosenReport = reportComboBox->currentIndex();
    const bool reportUsesPeriod = chosenReport == cashSalesIndex || chosenReport == allSalesIndex;
    periodComboBox->setEnabled(reportUsesPeriod);
    applyPeriodChoice();
}

void ReportsScreen::applyPeriodChoice()
{
    const QDate today = QDate::currentDate();
    const int chosenPeriod = periodComboBox->currentIndex();

    QDate startOfPeriod = today;
    if (chosenPeriod == weekPeriodIndex)
        startOfPeriod = today.addDays(-(today.dayOfWeek() - 1));
    else if (chosenPeriod == monthPeriodIndex)
        startOfPeriod = QDate(today.year(), today.month(), 1);
    else if (chosenPeriod == yearPeriodIndex)
        startOfPeriod = QDate(today.year(), 1, 1);

    if (chosenPeriod != specifiedPeriodIndex) {
        fromDateEdit->setDate(startOfPeriod);
        toDateEdit->setDate(today);
    }

    const bool datesCanBeEdited = periodComboBox->isEnabled() && chosenPeriod == specifiedPeriodIndex;
    fromDateEdit->setEnabled(datesCanBeEdited);
    toDateEdit->setEnabled(datesCanBeEdited);
}

void ReportsScreen::generateReport()
{
    const QDate fromDate = fromDateEdit->date();
    const QDate toDate = toDateEdit->date();
    const int chosenReport = reportComboBox->currentIndex();
    const bool reportUsesPeriod = chosenReport == cashSalesIndex || chosenReport == allSalesIndex;
    if (reportUsesPeriod && fromDate > toDate) {
        QMessageBox::warning(this, "Reports", "The start date must not be after the end date.");
        return;
    }

    ReportTable table;
    QString problemDescription;
    bool reportWasBuilt = false;
    if (chosenReport == incompleteOrdersIndex)
        reportWasBuilt = reportService.incompleteOrdersReport(table, problemDescription);
    else if (chosenReport == completedOrdersIndex)
        reportWasBuilt = reportService.completedOrdersReport(table, problemDescription);
    else if (chosenReport == cashSalesIndex)
        reportWasBuilt = reportService.cashSalesReport(fromDate, toDate, table, problemDescription);
    else if (chosenReport == allSalesIndex)
        reportWasBuilt = reportService.allSalesReport(fromDate, toDate, table, problemDescription);
    else if (chosenReport == topSoldIndex)
        reportWasBuilt = reportService.topSoldItemsReport(table, problemDescription);
    else if (chosenReport == topRemainingIndex)
        reportWasBuilt = reportService.topRemainingItemsReport(table, problemDescription);

    if (!reportWasBuilt) {
        QMessageBox::critical(this, "Reports", problemDescription);
        return;
    }
    showTable(table);
}

void ReportsScreen::showTable(const ReportTable& table){
    reportTable->clear();
    reportTable->setColumnCount(static_cast<int>(table.headers.size()));
    reportTable->setHorizontalHeaderLabels(table.headers);
    reportTable->setRowCount(static_cast<int>(table.rows.size()));
    for (size_t rowIndex = 0; rowIndex < table.rows.size(); ++rowIndex) {
        const QStringList& rowTexts = table.rows[rowIndex];
        for (int columnIndex = 0; columnIndex < rowTexts.size(); ++columnIndex)
            reportTable->setItem(static_cast<int>(rowIndex), columnIndex, new QTableWidgetItem(rowTexts[columnIndex]));
    }
    summaryLabel->setText(table.summary);
}
