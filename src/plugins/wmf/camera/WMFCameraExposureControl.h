#ifndef WMFCAMERAEXPOSURECONTROL_H
#define WMFCAMERAEXPOSURECONTROL_H

#include <qcamera.h>
#include <qcameraexposurecontrol.h>

QT_BEGIN_NAMESPACE

class WMFCameraSession;

class WMFCameraExposureControl : public QCameraExposureControl {
    Q_OBJECT

public:
    WMFCameraExposureControl(WMFCameraSession* session);
    virtual ~WMFCameraExposureControl() = default;

    QVariant actualValue(ExposureParameter parameter) const;
    bool isParameterSupported(ExposureParameter parameter) const;
    QVariant requestedValue(ExposureParameter parameter) const;
    bool setValue(ExposureParameter parameter, const QVariant& value);
    QVariantList supportedParameterRange(ExposureParameter parameter, bool* continuous) const;

private:
    WMFCameraSession* m_session;
};

QT_END_NAMESPACE

#endif // WMFCAMERAEXPOSURECONTROL_H
