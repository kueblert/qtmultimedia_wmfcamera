#ifndef WMFVIDEODEVICECONTROL_H
#define WMFVIDEODEVICECONTROL_H

#include <QDebug>
#include <QElapsedTimer>
#include <QStringList>
#include <QVideoDeviceSelectorControl>
#include "wmfcamerasession.h"

QT_BEGIN_NAMESPACE
typedef QPair<QByteArray, QString> WMFVideoDeviceInfo;

class WMFVideoDeviceControl: public QVideoDeviceSelectorControl
{
    Q_OBJECT
public:
    WMFVideoDeviceControl(QObject* parent = 0);
    ~WMFVideoDeviceControl() = default;


    int deviceCount() const;
    QString deviceName(int index) const;
    QString deviceDescription(int index) const;
    int defaultDevice() const;
    int selectedDevice() const;

    static const QList<WMFVideoDeviceInfo>& availableDevices();

public Q_SLOTS:
    void setSelectedDevice(int index);

private:

    static void updateDevices();

    WMFCameraSession* m_session;
    int selected;
};

QT_END_NAMESPACE

#endif // WMFVIDEODEVICECONTROL_H
