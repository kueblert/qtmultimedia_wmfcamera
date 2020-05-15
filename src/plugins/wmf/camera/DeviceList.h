#ifndef DEVICELIST_H
#define DEVICELIST_H

#include <windows.h>
#include <mfapi.h>

class DeviceList
{
    IMFActivate **m_ppDevices;
    UINT32      m_cDevices;

public:
    DeviceList() : m_ppDevices(NULL), m_cDevices(0)
    {

    }
    ~DeviceList()
    {
        Clear();
    }

    UINT32  Count() const { return m_cDevices; }

    void    Clear();
    HRESULT EnumerateDevices();
    HRESULT GetDevice(UINT32 index, IMFActivate **ppActivate);
    HRESULT GetDeviceName(UINT32 index, WCHAR **ppszName);
};

#endif // DEVICELIST_H
