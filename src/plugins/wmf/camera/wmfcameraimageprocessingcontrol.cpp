#include "WMFCameraImageProcessingControl.h"
#include "WMFCameraSession.h"

QT_BEGIN_NAMESPACE

WMFCameraImageProcessingControl::WMFCameraImageProcessingControl(WMFCameraSession* session)
    : QCameraImageProcessingControl(session)
    , m_session(session)
{
}

bool WMFCameraImageProcessingControl::isParameterSupported(
    QCameraImageProcessingControl::ProcessingParameter parameter) const
{
    return true;
}

bool WMFCameraImageProcessingControl::isParameterValueSupported(
    QCameraImageProcessingControl::ProcessingParameter parameter,
    const QVariant& value) const
{
    return false;
}

CAPTURE_PROPETIES WMFCameraImageProcessingControl::ProcessingParameter2WMF(ProcessingParameter p) const{
    switch(p){
        case ProcessingParameter::Brightness:
        return CAPTURE_BRIGHTNESS;
    case ProcessingParameter::Contrast:
    return CAPTURE_CONTRAST;
    case ProcessingParameter::ColorTemperature:
    return CAPTURE_HUE;
    case ProcessingParameter::Saturation:
    return CAPTURE_SATURATION;
    case ProcessingParameter::Sharpening:
    return CAPTURE_SHARPNESS;
    case ProcessingParameter::WhiteBalancePreset:
    return CAPTURE_WHITEBALANCE;    //TODO is that right?
    case ProcessingParameter::Denoising:
    return CAPTURE_BACKLIGHTCOMPENSATION; //TODO is that right?
    default:
        qWarning() << "Unknown Parameter";
        return CAPTURE_PROP_MAX;
    }
}

QVariant WMFCameraImageProcessingControl::parameter(
    QCameraImageProcessingControl::ProcessingParameter parameter) const
{
    float value;
    int autoValue;
    int devId = m_session->selectedDevice();
    m_session->m_backend->getRunningDevices()[devId]->getProperty(ProcessingParameter2WMF(parameter), value, autoValue);

    return QVariant(value);
}

void WMFCameraImageProcessingControl::setParameter(QCameraImageProcessingControl::ProcessingParameter parameter,
    const QVariant& value)
{

    int devId = m_session->selectedDevice();
    m_session->m_backend->getRunningDevices()[devId]->setProperty(ProcessingParameter2WMF(parameter), value.toFloat(), 0);

}

QT_END_NAMESPACE
