#pragma once

#include <QImage>
#include <QString>
#include <QWidget>

class CameraScanner;
class QLabel;
class QPushButton;

class ScanPanel : public QWidget {
    Q_OBJECT
public:
    explicit ScanPanel(QWidget* parent = nullptr);
    void stopScanning();

signals:
    void barcodeScanned(const QString& barcodeText);

private:
    QLabel* previewLabel = nullptr;
    QLabel* statusLabel = nullptr;
    QPushButton* cameraButton = nullptr;
    QPushButton* imageFileButton = nullptr;
    CameraScanner* cameraScanner = nullptr;

    void toggleCamera();
    void scanFromImageFile();
    void showPreview(const QImage& image);
    void updateCameraButtonText(bool cameraIsRunning);
};
