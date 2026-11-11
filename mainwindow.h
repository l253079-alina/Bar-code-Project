#pragma once

#include <QImage>
#include <QMainWindow>
#include <QString>

class QLabel;
class QLineEdit;

class MainWindow : public QMainWindow {
public:
    MainWindow();

private:
    QLabel* preview = nullptr;
    QLabel* resultLabel = nullptr;
    QLabel* barcodeView = nullptr;
    QLineEdit* numberEdit = nullptr;

    QString lastNumber;
    QImage generated;

    void showImage(const QImage& img);
    void showGenerated(const QString& n);
    void showResult(const QString& text);
    void openFile();
    void generate();
    void saveBarcode();
};
