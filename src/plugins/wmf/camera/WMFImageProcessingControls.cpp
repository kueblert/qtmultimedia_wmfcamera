#include "WMFImageProcessingControls.h"

WMFImageProcessingControls::WMFImageProcessingControls(WMFCameraSession *session)
    : QCameraImageProcessingControl()
    , m_session(session)
{

}

bool WMFImageProcessingControls::isParameterSupported(ProcessingParameter) const{
    return true;
}

bool WMFImageProcessingControls::isParameterValueSupported(ProcessingParameter parameter, const QVariant& value) const{
    return true;
}

QVariant WMFImageProcessingControls::parameter(ProcessingParameter parameter) const{
    return QVariant();
}

void WMFImageProcessingControls::setParameter(ProcessingParameter parameter, const QVariant& value){

}
