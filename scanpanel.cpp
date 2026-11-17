#include "scanpanel.h"

#include "barcodeUtils.h"
#include "camerascanner.h"

#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

ScanPanel::ScanPanel(QWidget* parent)
    : QWidget(parent)
{
    previewLabel = new QLabel("Camera preview");
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumSize(420, 320);
    previewLabel->setFrameShape(QFrame::StyledPanel);

    statusLabel = new QLabel("Press Start camera and hold the barcode in front of it.");
    statusLabel->setWordWrap(true);

    cameraButton = new QPushButton("Start camera");
    imageFileButton = new QPushButton("Scan from image file...");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(previewLabel, 1);
    layout->addWidget(statusLabel);
    layout->addWidget(cameraButton);
    layout->addWidget(imageFileButton);

    cameraScanner = new CameraScanner(this);
    connect(cameraScanner, &CameraScanner::frameReady, this, [this](const QImage& image) { showPreview(image); });
    connect(cameraScanner, &CameraScanner::barcodeFound, this, [this](const QString& barcodeText) {
        statusLabel->setText("Barcode read: " + barcodeText);
        emit barcodeScanned(barcodeText.trimmed());
    });
    connect(cameraScanner, &CameraScanner::errorOccurred, this, [this](const QString& message) {
        statusLabel->setText(message);
        QMessageBox::warning(this, "Camera", message);
    });
    connect(cameraScanner, &CameraScanner::runningChanged, this,
            [this](bool cameraIsRunning) { updateCameraButtonText(cameraIsRunning); });

    connect(cameraButton, &QPushButton::clicked, this, [this] { toggleCamera(); });
    connect(imageFileButton, &QPushButton::clicked, this, [this] { scanFromImageFile(); });
}

void ScanPanel::stopScanning()
{
    cameraScanner->stop();
}

void ScanPanel::toggleCamera()
{
    if (cameraScanner->isRunning()) {
        cameraScanner->stop();
        return;
    }
    if (cameraScanner->start())
        statusLabel->setText("Scanning... hold the barcode flat and fill about half of the picture.");
}

void ScanPanel::scanFromImageFile()
{
    const QString imagePath = QFileDialog::getOpenFileName(
        this, "Open barcode image", QString(), "Images (*.png *.jpg *.jpeg *.bmp *.gif *.webp)");
    if (imagePath.isEmpty())
        return;

    const QImage image(imagePath);
    if (image.isNull()) {
        QMessageBox::warning(this, "Error", "Could not load that image.");
        return;
    }

    showPreview(image);
    const optional<QString> barcodeText = decodeBarcode(image);
    if (!barcodeText.has_value()) {
        statusLabel->setText("No Code 128 barcode found in that image.");
        return;
    }
    statusLabel->setText("Barcode read: " + *barcodeText);
    emit barcodeScanned(barcodeText->trimmed());
}

void ScanPanel::showPreview(const QImage& image)
{
    previewLabel->setPixmap(QPixmap::fromImage(image).scaled(
        previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void ScanPanel::updateCameraButtonText(bool cameraIsRunning)
{
    if (cameraIsRunning)
        cameraButton->setText("Stop camera");
    else
        cameraButton->setText("Start camera");
}
