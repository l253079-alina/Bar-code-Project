// camerascanner.h - live camera scanning.

#pragma once

#include <QCamera>
#include <QElapsedTimer>
#include <QImage>
#include <QMediaCaptureSession>
#include <QObject>
#include <QString>
#include <QVideoSink>

class CameraScanner : public QObject {
    Q_OBJECT
public:
    explicit CameraScanner(QObject *parent = nullptr);

    bool isRunning() const { return camera != nullptr; }

    // Returns false (and emits errorOccurred) if no camera is available.
    bool start();
    void stop();

signals:
    void frameReady(const QImage &frame);      
    void barcodeFound(const QString &text);    
    void errorOccurred(const QString &message);
    void runningChanged(bool running);

private:
    QCamera *camera = nullptr;
    QVideoSink *sink = nullptr;
    QMediaCaptureSession session;
    QElapsedTimer throttle;
    QString pendingText;   
    int pendingCount = 0;
};
