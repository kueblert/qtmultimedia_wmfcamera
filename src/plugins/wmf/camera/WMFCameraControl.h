#ifndef WMFCAMERACONTROL_H
#define WMFCAMERACONTROL_H

#include <QtCore/qobject.h>
#include <QtMultimedia/QCameraControl>

QT_BEGIN_NAMESPACE

class WMFCameraSession;
class WMFCameraService;

class WMFCameraControl : public QCameraControl
{
public:
    WMFCameraControl(QObject* parent = 0);
    ~WMFCameraControl() = default;

    QCamera::State state() const { return m_state; }

    QCamera::CaptureModes captureMode() const { return m_captureMode; }
    void setCaptureMode(QCamera::CaptureModes mode);

    void setState(QCamera::State state);

    QCamera::Status status() const;
    bool isCaptureModeSupported(QCamera::CaptureModes mode) const;
    bool canChangeProperty(PropertyChangeType /* changeType */, QCamera::Status /* status */) const { return false; }

private:
    WMFCameraSession* m_session;
    WMFCameraService* m_service;
    QCamera::State m_state;
    QCamera::CaptureModes m_captureMode;
};

QT_END_NAMESPACE

#endif // WMFCAMERACONTROL_H
