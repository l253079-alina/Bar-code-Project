#pragma once

#include <QString>

struct OrderDetailRecord {
    QString orderId;
    QString productId;
    QString productBarcode;
    double quantity = 0.0;
};
