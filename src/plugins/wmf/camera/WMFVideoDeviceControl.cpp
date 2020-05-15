#include "WMFVideoDeviceControl.h"

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QList<WMFVideoDeviceInfo>, deviceList)


WMFVideoDeviceControl::WMFVideoDeviceControl(QObject* parent)
    : QVideoDeviceSelectorControl(parent)
    , selected(0)
{
    m_session = qobject_cast<WMFCameraSession*>(parent);
    updateDevices();
}

int WMFVideoDeviceControl::deviceCount() const
{
    updateDevices();
    return deviceList->count();
}

QString WMFVideoDeviceControl::deviceName(int index) const
{
    updateDevices();

    if (index >= 0 && index <= deviceList->count())
        return QString::fromUtf8(deviceList->at(index).first.constData());

    return QString();
}

QString WMFVideoDeviceControl::deviceDescription(int index) const
{
    updateDevices();

    if (index >= 0 && index <= deviceList->count())
        return deviceList->at(index).second;

    return QString();
}

int WMFVideoDeviceControl::defaultDevice() const
{
    return 0;
}

int WMFVideoDeviceControl::selectedDevice() const
{
    return selected;
}

void WMFVideoDeviceControl::setSelectedDevice(int index)
{
    updateDevices();

    if (index >= 0 && index < deviceList->count()) {
        if (m_session) {
            QString device = deviceList->at(index).first;
            if (m_session->setDevice(device))
                selected = index;
        }
    }
}

const QList<WMFVideoDeviceInfo>& WMFVideoDeviceControl::availableDevices()
{
    updateDevices();
    return *deviceList;
}

void WMFVideoDeviceControl::updateDevices()
{
    static QElapsedTimer timer;
    if (timer.isValid() && timer.elapsed() < 500) // ms
        return;

    deviceList->clear();

    DeviceList devicelist;
    devicelist.EnumerateDevices();
    WCHAR *szFriendlyName = NULL;

    for (UINT32 iDevice = 0; iDevice < devicelist.Count(); iDevice++)
    {
        // Get the friendly name of the device.
        HRESULT hr = devicelist.GetDeviceName(iDevice, &szFriendlyName);

        WMFVideoDeviceInfo devInfo;
        devInfo.first = QString::fromWCharArray(szFriendlyName).toUtf8();
        devInfo.second = devInfo.first;
        deviceList->append(devInfo);
    }

    timer.restart();
}
