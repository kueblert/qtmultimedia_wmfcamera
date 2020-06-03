#include "WMFCameraSession.h"
#include <QDebug>
#include <QMetaObject>

WMFCameraSession::WMFCameraSession(WMFCameraBackend* backend, QObject* parent)
    :QObject(nullptr)
    , m_status(QCamera::UnloadedStatus)
    , m_surface(nullptr)
    , m_selectedDevice(0)
    , m_backend(backend)
{
    updateSourceCapabilities();
}

void WMFCameraSession::updateSourceCapabilities(){
    //TODO read from devices
    m_supportedViewfinderSettings.clear();
    QCameraViewfinderSettings settingsField;
    settingsField.setResolution(640, 480);
    settingsField.setPixelAspectRatio(1, 1);
    settingsField.setPixelFormat(QVideoFrame::Format_Jpeg);
    settingsField.setMinimumFrameRate(60);
    settingsField.setMaximumFrameRate(60);
    m_supportedViewfinderSettings.push_back(settingsField);

    QCameraViewfinderSettings settingsField2;
    settingsField2.setResolution(640, 480);
    settingsField2.setPixelAspectRatio(1, 1);
    settingsField2.setPixelFormat(QVideoFrame::Format_NV12);
    settingsField2.setMinimumFrameRate(60);
    settingsField2.setMaximumFrameRate(60);
    m_supportedViewfinderSettings.push_back(settingsField2);

    QCameraViewfinderSettings settingsEye;
    settingsEye.setResolution(320, 240);
    settingsEye.setPixelAspectRatio(1, 1);
    settingsEye.setPixelFormat(QVideoFrame::Format_Jpeg);
    settingsEye.setMinimumFrameRate(60);
    settingsEye.setMaximumFrameRate(60);
    m_supportedViewfinderSettings.push_back(settingsEye);

    QCameraViewfinderSettings settingsEye2;
    settingsEye2.setResolution(320, 240);
    settingsEye2.setPixelAspectRatio(1, 1);
    settingsEye2.setPixelFormat(QVideoFrame::Format_NV12);
    settingsEye2.setMinimumFrameRate(60);
    settingsEye2.setMaximumFrameRate(60);
    m_supportedViewfinderSettings.push_back(settingsEye2);
}

void WMFCameraSession::presentFrame(const QVideoFrame& frame)
{
    if (m_surface){
        bool ok = m_surface->present(frame);
        if(!ok){
            qWarning() << "Frame presentation failed:";
            qWarning() << frame;
            qDebug() << "stopPreview" << stopPreview();
            qDebug() << "unload: " << unload();
            qDebug() << "load: " << load();
            qDebug() << "startPreview" << startPreview();
        }
    }
    else{
        qWarning() << "QAbstractVideoSurface not ready";
    }

}

void WMFCameraSession::presentNewFrame(int device, const QVideoFrame& frame)
{
    if(device != m_selectedDevice) return;
    presentFrame(frame);
}

bool WMFCameraSession::startPreview(){
    qDebug() << m_settings.pixelFormat() << "@" << m_settings.resolution().width() << "x" << m_settings.resolution().height();
    assert(m_settings.pixelFormat() != QVideoFrame::Format_Invalid);
    QMetaObject::invokeMethod(m_backend, "setResolution", Qt::QueuedConnection, Q_ARG(int, m_selectedDevice), Q_ARG(int, m_settings.resolution().width()), Q_ARG(int,  m_settings.resolution().height()), Q_ARG(QVideoFrame::PixelFormat, m_settings.pixelFormat()));

    connect(m_backend, &WMFCameraBackend::newFrame, this, &WMFCameraSession::presentNewFrame);
    QMetaObject::invokeMethod(m_backend, "requestDevice", Qt::QueuedConnection, Q_ARG(int, m_selectedDevice));
    return true;
}

bool WMFCameraSession::stopPreview(){
    disconnect(m_backend, &WMFCameraBackend::newFrame, this, &WMFCameraSession::presentNewFrame);
    QMetaObject::invokeMethod(m_backend, "freeDevice", Qt::QueuedConnection, Q_ARG(int, m_selectedDevice));
    return true;
}

bool WMFCameraSession::load(){
    return true;
}

void WMFCameraSession::deviceNotificationCallback(bool isArrival){
    qDebug() << "Device changed" << isArrival;
}

bool WMFCameraSession::unload(){
    return true;
}

void WMFCameraSession::setSelectedDevice(int index){
    //TODO check whether we are already streaming
    m_selectedDevice = index;
}

QString WMFCameraSession::deviceName(){
    return m_backend->getDeviceNames()[m_selectedDevice];
}
