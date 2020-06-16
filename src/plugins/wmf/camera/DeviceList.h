#ifndef DEVICELIST_H
#define DEVICELIST_H

#include <windows.h>
#include <mfapi.h>
#include <QMutex>
#include <QStringList>

class DeviceList
{
    IMFActivate **m_ppDevices;
    UINT32      m_cDevices;

public:
    DeviceList();
    ~DeviceList()
    {
        Clear();
    }

    UINT32  Count() const { return m_cDevices; }

    // Clear and EnumerateDevices needs to be threadsafe and callable from different threads simultaneously
    void    Clear();
    HRESULT EnumerateDevices();

    HRESULT GetDevice(UINT32 index, IMFActivate **ppActivate);
    HRESULT GetDeviceName(UINT32 index, WCHAR **ppszName);

private:
    QMutex m_enumerationMutex;

    // We are caching this to be able to handle cameras with the same name
    void cacheDeviceNames();
    QStringList deviceNames;
};

#endif // DEVICELIST_H
