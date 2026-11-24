#pragma once
#include <QString>
class QWidget;

class ManagerApprovalService 
{
public:
    explicit ManagerApprovalService(const QString& dataFolderPathToUse);
    bool askForApproval(QWidget* parentWidget) const;

private:
    QString dataFolderPath;
    QString expectedPin() const;
};
