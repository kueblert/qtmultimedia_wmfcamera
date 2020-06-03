#ifndef ESCAPICAMERASERVICE_H
#define ESCAPICAMERASERVICE_H

#include <QMediaService>
#include "WMFCameraBackend.h"
#include "WMFCameraSession.h"
#include "WMFVideoRendererControl.h"
#include "WMFViewfinderSettingsControl2.h"
#include "WMFVideoDeviceSelectorControl.h"
#include "WMFImageProcessingControls.h"
#include "WMFCameraControl.h"
#include "WMFCameraExposureControl.h"
#include "WMFCameraImageProcessingControl.h"

class WMFCameraService : public QMediaService

{
    Q_OBJECT
public:
    WMFCameraService(WMFCameraBackend* backend, QObject* parent = nullptr);
    ~WMFCameraService();

    // QMediaService
    virtual void releaseControl(QMediaControl *control);
    virtual QMediaControl *	requestControl(const char *interfaceName);

private:
    WMFCameraBackend* backend;
    WMFCameraSession* m_session;
    WMFCameraControl* m_control;
    WMFVideoRendererControl *m_videoRenderer;
    WMFViewfinderSettingsControl2* m_viewfinder;
    WMFVideoDeviceSelectorControl* m_deviceSelector;
    WMFImageProcessingControls* m_imageProcessing;
    WMFCameraImageProcessingControl *m_imageProcessingControl;
    WMFCameraExposureControl *m_cameraExposureControl;
};

#endif // ESCAPICAMERASERVICE_H
