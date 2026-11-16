#pragma once
#include "bill.h"
#include <QDialog>

class BillDialog : public QDialog {
public:
    explicit BillDialog(const Bill& bill, QWidget* parent = nullptr);
};
