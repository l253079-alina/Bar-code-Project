#pragma once

#include <QString>

namespace OrderConstants {
    inline const QString statusInProgress = "in-progress";
    inline const QString statusOnHold = "on-hold";
    inline const QString statusPaid = "paid";
    inline const QString statusArchived = "archived";

    inline const QString paymentModeCash = "cash";
    inline const QString paymentModeCard = "card";
    inline const QString paymentModeWallet = "wallet";

    inline const QString measurementUnitCount = "count";
    inline const QString measurementUnitWeight = "weight";

    inline const QString dateFormat = "yyyy-MM-dd";
    inline const QString timeFormat = "HH:mm:ss";

    inline const int paidOrderEditWindowDays = 7;
    inline const double maximumSpecialDiscountPercent = 8.0;

    inline bool isCompletedStatus(const QString& status)
    {
        return status == statusPaid || status == statusArchived;
    }
}
