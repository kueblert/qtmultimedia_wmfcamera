#include "WMFCameraExposureControl.h"
#include "WMFCameraSession.h"

QT_BEGIN_NAMESPACE

WMFCameraExposureControl::WMFCameraExposureControl(WMFCameraSession* session)
    : QCameraExposureControl(session)
    , m_session(session)
{
}

QVariant WMFCameraExposureControl::actualValue(ExposureParameter parameter) const
{
    float value;
    int autoValue;
    int devId = m_session->selectedDevice();
    m_session->m_backend->getRunningDevices()[devId]->getProperty(CAPTURE_EXPOSURE, value, autoValue);

    return QVariant(value);
}

bool WMFCameraExposureControl::isParameterSupported(ExposureParameter parameter) const
{
    return true;
}

QVariant WMFCameraExposureControl::requestedValue(ExposureParameter parameter) const
{
    float value;
    int autoValue;
    int devId = m_session->selectedDevice();
    m_session->m_backend->getRunningDevices()[devId]->getProperty(CAPTURE_EXPOSURE, value, autoValue);

    return QVariant(value);
}

bool WMFCameraExposureControl::setValue(ExposureParameter parameter, const QVariant& value)
{
    int devId = m_session->selectedDevice();
    m_session->m_backend->getRunningDevices()[devId]->setProperty(CAPTURE_EXPOSURE, value.toFloat(), 0);

    return true;
}

QVariantList WMFCameraExposureControl::supportedParameterRange(ExposureParameter parameter, bool* continuous) const
{
    return QVariantList();
}

QT_END_NAMESPACE
