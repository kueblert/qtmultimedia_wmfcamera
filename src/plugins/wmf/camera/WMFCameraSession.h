#ifndef ESCAPICAMERASESSION_H
#define ESCAPICAMERASESSION_H

#include <QObject>
#include <QMutex>
#include <QCamera>
#include <QTimer>

#include <QCameraViewfinderSettingsControl2>

#include <QAbstractVideoSurface>

//#include "escapi.h"
#include "WMFCameraBackend.h"

class WMFCameraSession: public QObject
{
    Q_OBJECT

public:
    WMFCameraSession(WMFCameraBackend* backend, QObject* parent = nullptr);



    // QCameraViewfinderSettingsControl2
    void setViewfinderSettings(const QCameraViewfinderSettings &settings) {
        m_settings = settings;
    }
    QList<QCameraViewfinderSettings>	supportedViewfinderSettings() const {
        return m_supportedViewfinderSettings;
    }
    QCameraViewfinderSettings	viewfinderSettings() const {
        return m_settings;
    }

    // interface for QVideoRendererControl
    void setSurface(QAbstractVideoSurface *surface) {
        QMutexLocker locker(&surfaceMutex);
        m_surface = surface;
    }

    QAbstractVideoSurface *	surface() const { return m_surface; }

    QString deviceName();

    void setSelectedDevice(int index);
    int selectedDevice() { return m_selectedDevice; }

    bool startPreview();
    bool stopPreview();
    bool load();
    bool unload();
    QCamera::Status status() {return m_status; }

    WMFCameraBackend* m_backend;

   private slots:
    void presentFrame(const QVideoFrame& frame);
    void presentNewFrame(int device, const QVideoFrame& frame);



private:

    int m_selectedDevice;

    QAbstractVideoSurface* m_surface;
    QMutex surfaceMutex;

    void updateSourceCapabilities();
    QCameraViewfinderSettings m_settings;
    QList<QCameraViewfinderSettings> m_supportedViewfinderSettings;

    QCamera::Status m_status;

    static void deviceNotificationCallback(bool isArrival);

};

#endif // ESCAPICAMERASESSION_H
