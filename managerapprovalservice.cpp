#include "managerapprovalservice.h"
#include "textfilelines.h"
#include <QDir>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QStringList>
static const QString pinFileName = "ManagerPin.txt";
static const QString defaultPin = "1234";

ManagerApprovalService::ManagerApprovalService(const QString& dataFolderPathToUse): dataFolderPath(dataFolderPathToUse)
{}

QString ManagerApprovalService::expectedPin() const
{
    const QStringList pinLines = TextFileLines::readNonEmptyLines(QDir(dataFolderPath).filePath(pinFileName));
    if (pinLines.isEmpty())
        return defaultPin;
    return pinLines.first();
}
bool ManagerApprovalService::askForApproval(QWidget* parentWidget) const
{
    bool pinWasEntered = false;
    const QString enteredPin = QInputDialog::getText(parentWidget, "Manager approval", "This order was already paid. A manager must approve the change.\nManager PIN:",
QLineEdit::Password, QString(), &pinWasEntered);
    if (!pinWasEntered)
        return false;
    if (enteredPin.trimmed() != expectedPin()) 
    {
        QMessageBox::warning(parentWidget, "Manager approval", "The PIN is not correct.");
        return false;
    }
    return true;
}
