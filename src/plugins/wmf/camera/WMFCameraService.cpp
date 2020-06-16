#include "WMFCameraService.h"
#include "WMFCameraSession.h"


WMFCameraService::WMFCameraService(WMFCameraBackend* backend, QObject *parent)
    : QMediaService(parent)
    , m_session(nullptr)
    , m_control(nullptr)
    , m_videoRenderer(nullptr)
    , backend(backend)
    , m_imageProcessing(nullptr)
{

    m_session = new WMFCameraSession(backend);
    m_viewfinder = new WMFViewfinderSettingsControl2(m_session);
    m_deviceSelector = new WMFVideoDeviceSelectorControl(m_session);
    m_imageProcessing = new WMFImageProcessingControls(m_session);
    m_control = new WMFCameraControl(m_session);
    m_imageProcessingControl = new WMFCameraImageProcessingControl(m_session);
    m_cameraExposureControl = new WMFCameraExposureControl(m_session);
}

WMFCameraService::~WMFCameraService(){

    delete m_viewfinder;
    delete m_deviceSelector;
    delete m_imageProcessing;
    delete m_control;
    delete m_imageProcessingControl;
    delete m_cameraExposureControl;

    if(m_videoRenderer)
        delete m_videoRenderer;

    // session must be last since it's used by the rest
    delete m_session;
}

void WMFCameraService::releaseControl(QMediaControl *control){
    if (control == m_videoRenderer) {
        delete m_videoRenderer;
        m_videoRenderer = nullptr;
        return;
    }
}

QMediaControl *	WMFCameraService::requestControl(const char *interfaceName)
{
    if(qstrcmp(interfaceName, QCameraControl_iid) == 0)
        return m_control;

    if(qstrcmp(interfaceName, QVideoRendererControl_iid) == 0){
        if(!m_videoRenderer){
            m_videoRenderer = new WMFVideoRendererControl(m_session, this);
            return m_videoRenderer;
        }
    }

    if (qstrcmp(interfaceName,QVideoDeviceSelectorControl_iid) == 0)
        return m_deviceSelector;

    if(qstrcmp(interfaceName, QCameraViewfinderSettingsControl2_iid) == 0)
        return m_viewfinder;

    if(qstrcmp(interfaceName, QCameraImageProcessingControl_iid) == 0){
        return m_imageProcessing;
    }

    if (qstrcmp(interfaceName, QCameraImageProcessingControl_iid) == 0)
        return m_imageProcessingControl;

    if (qstrcmp(interfaceName, QCameraExposureControl_iid) == 0)
        return m_cameraExposureControl;

    //qDebug() << "requestControl unavailable: " << interfaceName;
    return nullptr;
}

