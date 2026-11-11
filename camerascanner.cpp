#include "camerascanner.h"

#include "barcodeUtils.h"

#include <QMediaDevices>
#include <QVideoFrame>

CameraScanner::CameraScanner(QObject *parent) : QObject(parent) {}

bool CameraScanner::start()
{
    if (camera)
        return true;
    if (QMediaDevices::videoInputs().isEmpty()) {
        emit errorOccurred("No camera found.");
        return false;
    }

    camera = new QCamera(QMediaDevices::defaultVideoInput(), this);
    sink = new QVideoSink(this);
    session.setCamera(camera);
    session.setVideoSink(sink);
    throttle.start();
    pendingText.clear();
    pendingCount = 0;

    connect(sink, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame &frame) {
        if (!frame.isValid())
            return;
        const QImage img = frame.toImage();
        if (img.isNull())
            return;
        emit frameReady(img);
        if (throttle.elapsed() < 120)          // decode ~8 times per second
            return;
        throttle.restart();

        if (auto text = decodeBarcode(img)) {
            pendingCount = (*text == pendingText) ? pendingCount + 1 : 1;
            pendingText = *text;
            if (pendingCount >= 2) {          
                const QString found = *text;
                emit barcodeFound(found);
                stop();
            }
        }
    });

    camera->start();
    emit runningChanged(true);
    return true;
}

void CameraScanner::stop()
{
    if (!camera)
        return;
    camera->stop();
    session.setVideoSink(nullptr);
    session.setCamera(nullptr);
    camera->deleteLater();
    sink->deleteLater();
    camera = nullptr;
    sink = nullptr;
    emit runningChanged(false);
}
