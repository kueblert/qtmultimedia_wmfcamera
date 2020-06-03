#ifndef ESCAPICAMERACONTROL_H
#define ESCAPICAMERACONTROL_H

#include <QCameraControl>

class WMFCameraSession;
class WMFCameraControl: public QCameraControl
{
    Q_OBJECT
public:
    WMFCameraControl(WMFCameraSession* session);

    // QCameraControl
    QCamera::State state() const override { return m_state; }

    QCamera::CaptureModes captureMode() const override { return m_captureMode; }
    void setCaptureMode(QCamera::CaptureModes mode) override;

    void setState(QCamera::State state) override;

    QCamera::Status status() const override;
    bool isCaptureModeSupported(QCamera::CaptureModes mode) const override;
    bool canChangeProperty(PropertyChangeType /* changeType */, QCamera::Status /* status */) const override { return false; }

private:
    QCamera::State m_state;
    QCamera::CaptureModes m_captureMode;

    WMFCameraSession* m_session;
};

#endif // ESCAPICAMERACONTROL_H
