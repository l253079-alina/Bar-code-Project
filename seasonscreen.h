#pragma once

#include "seasoncatalog.h"

#include <QWidget>

class QCheckBox;
class QDateEdit;
class QDoubleSpinBox;
class QLabel;
class QTableWidget;

class SeasonScreen : public QWidget {
public:
    explicit SeasonScreen(SeasonCatalog& catalog, QWidget* parent = nullptr);
    void refresh();

private:
    SeasonCatalog& seasonCatalog;

    QTableWidget* seasonTable = nullptr;
    QDateEdit* startDateEdit = nullptr;
    QDateEdit* endDateEdit = nullptr;
    QDoubleSpinBox* discountPercentSpinBox = nullptr;
    QCheckBox* activeCheckBox = nullptr;
    QLabel* messageLabel = nullptr;

    void addSeason();
    void toggleSelectedSeason();
};
