#include "WMFCameraViewfinderSettingsControl.h"
#include "wmfcamerasession.h"

QT_BEGIN_NAMESPACE

WMFCameraViewfinderSettingsControl::WMFCameraViewfinderSettingsControl(WMFCameraSession *session)
    :m_session(session)
{

}

QList<QCameraViewfinderSettings> WMFCameraViewfinderSettingsControl::supportedViewfinderSettings() const
{
    return m_session->supportedViewfinderSettings();
}

QCameraViewfinderSettings WMFCameraViewfinderSettingsControl::viewfinderSettings() const
{
    return m_session->viewfinderSettings();
}

void WMFCameraViewfinderSettingsControl::setViewfinderSettings(const QCameraViewfinderSettings &settings)
{
    m_session->setViewfinderSettings(settings);
}


QT_END_NAMESPACE
