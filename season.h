#pragma once

#include <QDate>

struct Season {
    QDate startDate;
    QDate endDate;
    double discountPercent = 0.0;
    bool isActive = false;
};
