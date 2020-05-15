#ifndef WMFCAMERAVIEWFINDERSETTINGSCONTROL_H
#define WMFCAMERAVIEWFINDERSETTINGSCONTROL_H

#include <qcameraviewfindersettingscontrol.h>

QT_BEGIN_NAMESPACE

class WMFCameraSession;

class WMFCameraViewfinderSettingsControl: public QCameraViewfinderSettingsControl2
{
public:
    WMFCameraViewfinderSettingsControl(WMFCameraSession *session);

    QList<QCameraViewfinderSettings> supportedViewfinderSettings() const;

    QCameraViewfinderSettings viewfinderSettings() const;
    void setViewfinderSettings(const QCameraViewfinderSettings &settings);

private:
    WMFCameraSession *m_session;
};

QT_END_NAMESPACE

#endif // WMFCAMERAVIEWFINDERSETTINGSCONTROL_H
