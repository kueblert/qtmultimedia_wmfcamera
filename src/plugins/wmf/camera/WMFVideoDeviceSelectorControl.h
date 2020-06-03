#ifndef ESCAPIVIDEODEVICESELECTORCONTROL_H
#define ESCAPIVIDEODEVICESELECTORCONTROL_H

#include <QVideoDeviceSelectorControl>
typedef QPair<QByteArray, QString> ESCAPIVideoDeviceInfo;
class WMFCameraSession;

class WMFVideoDeviceSelectorControl :
        public QVideoDeviceSelectorControl
{
    Q_OBJECT
public:
    WMFVideoDeviceSelectorControl(WMFCameraSession* session, QObject* parent = nullptr);

    // QVideoDeviceSelectorControl
    virtual int	defaultDevice() const { return 0; }
    virtual int	deviceCount() const;
    virtual QString	deviceDescription(int index) const;
    virtual QString	deviceName(int index) const;
    virtual int	selectedDevice() const;
    static const QList<ESCAPIVideoDeviceInfo>& availableDevices();
public Q_SLOTS:
    void setSelectedDevice(int index);

private:
    WMFCameraSession* m_session;
    int nDevices;

    static void updateDevices();

};

#endif // ESCAPIVIDEODEVICESELECTORCONTROL_H
