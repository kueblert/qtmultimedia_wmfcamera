#ifndef ESCAPIIMAGEPROCESSINGCONTROLS_H
#define ESCAPIIMAGEPROCESSINGCONTROLS_H

#include <QCameraImageProcessingControl>

class WMFCameraSession;

class WMFImageProcessingControls : public QCameraImageProcessingControl
{
    Q_OBJECT
public:
    WMFImageProcessingControls(WMFCameraSession* session);

    bool isParameterSupported(ProcessingParameter) const;
    bool isParameterValueSupported(ProcessingParameter parameter, const QVariant& value) const;
    QVariant parameter(ProcessingParameter parameter) const;
    void setParameter(ProcessingParameter parameter, const QVariant& value);

private:
    WMFCameraSession* m_session;
};

#endif // ESCAPIIMAGEPROCESSINGCONTROLS_H
