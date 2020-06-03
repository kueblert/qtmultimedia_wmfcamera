#include "WMFViewfinderSettingsControl2.h"
#include "WMFCameraSession.h"

WMFViewfinderSettingsControl2::WMFViewfinderSettingsControl2(WMFCameraSession* session, QObject* parent)
    : QCameraViewfinderSettingsControl2(parent)
    , m_session(session)
{

}

// QCameraViewfinderSettingsControl2
void WMFViewfinderSettingsControl2::setViewfinderSettings(const QCameraViewfinderSettings &settings) {
    m_session->setViewfinderSettings(settings);
}
QList<QCameraViewfinderSettings> WMFViewfinderSettingsControl2::supportedViewfinderSettings() const {
    return m_session->supportedViewfinderSettings();
}
QCameraViewfinderSettings WMFViewfinderSettingsControl2::viewfinderSettings() const {
    return m_session->viewfinderSettings();
}
