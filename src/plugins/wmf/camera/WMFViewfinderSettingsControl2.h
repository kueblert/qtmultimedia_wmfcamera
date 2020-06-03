#ifndef ESCAPIVIEWFINDERSETTINGSCONTROL2_H
#define ESCAPIVIEWFINDERSETTINGSCONTROL2_H

#include <QCameraViewfinderSettingsControl2>

class WMFCameraSession;

class WMFViewfinderSettingsControl2 : public QCameraViewfinderSettingsControl2
{
public:
    WMFViewfinderSettingsControl2(WMFCameraSession* session, QObject* parent = nullptr);

    // QCameraViewfinderSettingsControl2
    virtual void setViewfinderSettings(const QCameraViewfinderSettings &settings) override;
    virtual QList<QCameraViewfinderSettings>	supportedViewfinderSettings() const override;
    virtual QCameraViewfinderSettings	viewfinderSettings() const override;

private: WMFCameraSession* m_session;
};

#endif // ESCAPIVIEWFINDERSETTINGSCONTROL2_H
