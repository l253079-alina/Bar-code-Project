#pragma once

#include "reportservice.h"

#include <QWidget>

class QComboBox;
class QDateEdit;
class QLabel;
class QTableWidget;

class ReportsScreen : public QWidget {
public:
    explicit ReportsScreen(ReportService& service, QWidget* parent = nullptr);

private:
    ReportService& reportService;

    QComboBox* reportComboBox = nullptr;
    QComboBox* periodComboBox = nullptr;
    QDateEdit* fromDateEdit = nullptr;
    QDateEdit* toDateEdit = nullptr;
    QTableWidget* reportTable = nullptr;
    QLabel* summaryLabel = nullptr;

    void updatePeriodControls();
    void applyPeriodChoice();
    void generateReport();
    void showTable(const ReportTable& table);
};
