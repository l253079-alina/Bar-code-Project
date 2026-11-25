#include "camerascanner.h"

#include "barcodeUtils.h"

#include <QMediaDevices>
#include <QVideoFrame>

static const int minimumMillisecondsBetweenDecodes = 120;
static const int sameResultsRequiredToConfirm = 2;
static const int sameBarcodeAbsenceMilliseconds = 1500;

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
    hasConfirmedText = false;
    lastSeenTimer.start();

    connect(sink, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame &frame) {
        if (!frame.isValid())
            return;
        const QImage frameImage = frame.toImage();
        if (frameImage.isNull())
            return;
        emit frameReady(frameImage);
        if (throttle.elapsed() < minimumMillisecondsBetweenDecodes)
            return;
        throttle.restart();

        if (auto barcodeText = decodeBarcode(frameImage)) {
            const bool isRepeatOfConfirmed = hasConfirmedText && *barcodeText == lastConfirmedText;
            if (isRepeatOfConfirmed) {
                const bool wasAbsentLongEnough = lastSeenTimer.elapsed() >= sameBarcodeAbsenceMilliseconds;
                lastSeenTimer.restart();
                if (!wasAbsentLongEnough) {
                    pendingText.clear();
                    pendingCount = 0;
                    return;
                }
            }

            if (*barcodeText == pendingText)
                pendingCount = pendingCount + 1;
            else
                pendingCount = 1;
            pendingText = *barcodeText;
            if (pendingCount >= sameResultsRequiredToConfirm) {
                const QString confirmedText = *barcodeText;
                lastConfirmedText = confirmedText;
                hasConfirmedText = true;
                lastSeenTimer.restart();
                pendingText.clear();
                pendingCount = 0;
                emit barcodeFound(confirmedText);
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
