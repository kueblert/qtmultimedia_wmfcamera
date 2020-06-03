#ifndef WMFCAMERAIMAGEPROCESSINGCONTROL_H
#define WMFCAMERAIMAGEPROCESSINGCONTROL_H

#include <qcamera.h>
#include <qcameraimageprocessingcontrol.h>
#include "escapi_minimal.h"

QT_BEGIN_NAMESPACE

class WMFCameraSession;

class WMFCameraImageProcessingControl : public QCameraImageProcessingControl {
    Q_OBJECT

public:
    WMFCameraImageProcessingControl(WMFCameraSession* session);
    virtual ~WMFCameraImageProcessingControl() = default;

    bool isParameterSupported(ProcessingParameter) const;
    bool isParameterValueSupported(ProcessingParameter parameter, const QVariant& value) const;
    QVariant parameter(ProcessingParameter parameter) const;
    void setParameter(ProcessingParameter parameter, const QVariant& value);

private:
    WMFCameraSession* m_session;

    CAPTURE_PROPETIES ProcessingParameter2WMF(ProcessingParameter p) const;
};

QT_END_NAMESPACE

#endif // WMFCAMERAIMAGEPROCESSINGCONTROL_H
