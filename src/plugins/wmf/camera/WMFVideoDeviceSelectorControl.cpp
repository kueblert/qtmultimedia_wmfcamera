#include "WMFVideoDeviceSelectorControl.h"
#include "WMFCameraSession.h"
#include "DeviceList.h"

Q_GLOBAL_STATIC(QList<ESCAPIVideoDeviceInfo>, deviceList)

WMFVideoDeviceSelectorControl::WMFVideoDeviceSelectorControl(WMFCameraSession *session, QObject *parent)
    : QVideoDeviceSelectorControl(parent)
    , m_session(session)
{

}

int	WMFVideoDeviceSelectorControl::selectedDevice() const {
    return m_session->selectedDevice();
}

void WMFVideoDeviceSelectorControl::setSelectedDevice(int index){ m_session->setSelectedDevice(index); }

QString	WMFVideoDeviceSelectorControl::deviceDescription(int index) const{
    return m_session->m_backend->getDeviceNames()[index];
    //return deviceList->at(index).first;
}

int	WMFVideoDeviceSelectorControl::deviceCount() const {
    return m_session->m_backend->getDeviceNames().size();
}

QString	WMFVideoDeviceSelectorControl::deviceName(int index) const{
    return m_session->m_backend->getDeviceNames()[index];
    //return deviceList->at(index).first;
}

const QList<ESCAPIVideoDeviceInfo>& WMFVideoDeviceSelectorControl::availableDevices()
{
    updateDevices();
    return *deviceList;
}

void WMFVideoDeviceSelectorControl::updateDevices()
{
    deviceList->clear();
    DeviceList dl;
    dl.EnumerateDevices();
    WCHAR *szFriendlyName = NULL;

    for(uint i = 0; i < dl.Count(); i++){
        ESCAPIVideoDeviceInfo info;
        dl.GetDeviceName(i, &szFriendlyName);
        info.second = QString::fromWCharArray(szFriendlyName);
        info.first = info.second.toUtf8();
        deviceList->append(info);
    }
}
