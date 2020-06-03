#ifndef ESCAPIBACKEND_H
#define ESCAPIBACKEND_H
#include <QList>
#include <QVideoFrame>
#include <QMutex>
#include <mfapi.h>
#include <mfplay.h>
#include "DeviceList.h"
#include "capture.h"
#include <QMap>
#include "escapi_minimal.h"

class WMFCameraBackend : public QObject
{
    Q_OBJECT
public:
    WMFCameraBackend();
    ~WMFCameraBackend();

    Q_INVOKABLE void init();

    QList<QByteArray> getDeviceNames();
    void setProperties(int devId, SimpleCapParams param);

    QMap<int, CaptureClass*> getRunningDevices() { return m_runningDevices; }

public slots:

    void requestDevice(int devId);
    void freeDevice(int devId);
    void setResolution(int devId, int width, int height, const QVideoFrame::PixelFormat &format);

signals:
    void deviceStateChanged(int devId, bool available, QByteArray name, QByteArray description);
    void newFrame(int devId, const QVideoFrame& f);

private:
    QMap<int, CaptureClass*> m_runningDevices;
    QMap<int, SimpleCapParams>* gParams;


    QMutex m_devicePorps;


    // MF stuff
    bool initApp();
    int setup(); // returns number of found cameras
    DeviceList dl;
    void CheckForFail(int devId);
    float GetProperty(int aDevice, int aProp);
    int GetPropertyAuto(int aDevice, int aProp);
    int SetProperty(int aDevice, int aProp, float aValue, int aAutoval);

private slots:
    void newFrameReceived(int devId);

};

#endif // ESCAPIBACKEND_H
