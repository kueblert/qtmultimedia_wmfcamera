#include "DeviceList.h"
#include <mfidl.h>
#include <QDebug>
#include <system_error>
#include <QMutexLocker>


template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

DeviceList::DeviceList()
    : m_ppDevices(NULL)
    , m_cDevices(0)
{
}

void DeviceList::Clear()
{
    QMutexLocker lock(&m_enumerationMutex);
    for (UINT32 i = 0; i < m_cDevices; i++)
    {
        SafeRelease(&m_ppDevices[i]);
    }
    CoTaskMemFree(m_ppDevices);
    m_ppDevices = NULL;

    m_cDevices = 0;
}

HRESULT DeviceList::EnumerateDevices()
{


    Clear();

    QMutexLocker lock(&m_enumerationMutex);
    IMFAttributes *pAttributes = NULL;
    // Initialize an attribute store. We will use this to
    // specify the enumeration parameters.
    HRESULT hr = MFCreateAttributes(&pAttributes, 1);

    // Ask for source type = video capture devices
    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetGUID(
                    MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                    MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
                    );
    }
    else{
        qWarning() << "Error creating attributes";
        return hr;
    }


    // Enumerate devices.
    if (SUCCEEDED(hr))
    {
        hr = MFEnumDeviceSources(pAttributes, &m_ppDevices, &m_cDevices);
    }
    else{
        qWarning() << "Unable to set attributes filter";
    }

    if(FAILED(hr)){
        qWarning() << "Error enumerating devices";
    }

    lock.unlock(); // We need to unlock in order to cache
    cacheDeviceNames();

    SafeRelease(&pAttributes);

    return hr;
}

void DeviceList::cacheDeviceNames(){
    //qDebug() << "caching";
    // make sure no clear can be called after we checked the index.
    QMutexLocker lock(&m_enumerationMutex);

    deviceNames.clear();

    HRESULT hr = S_OK;
    for(uint i = 0; i < Count(); i++){
        //qDebug() << i;
        UINT32 cchName;
        WCHAR *szFriendlyName = NULL;
        hr = m_ppDevices[i]->GetAllocatedString(
                    MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                    &szFriendlyName,
                    &cchName
                    );
        if(FAILED(hr)) continue;
        QString baseName = QString::fromWCharArray(szFriendlyName);
        //qDebug() << "base " << baseName;
        QString insertName = baseName;
        int j = 1;
        while(deviceNames.contains(insertName)){
            //qDebug() << "trying " << insertName;
            insertName = baseName + " #" + QString::number(j);
            j++;
        }
        //qDebug() << "caching " << insertName;
        deviceNames.push_back(insertName);
        CoTaskMemFree(szFriendlyName);
        //delete[] szFriendlyName; //or CoTaskMemFree ?
    }

    //qDebug() << "caching done";
}


HRESULT DeviceList::GetDevice(UINT32 index, IMFActivate **ppActivate)
{
    // make sure no clear can be called after we checked the index.
    QMutexLocker lock(&m_enumerationMutex);
    if (index >= Count())
    {
        return E_INVALIDARG;
    }

    *ppActivate = m_ppDevices[index];
    (*ppActivate)->AddRef();

    return S_OK;
}

HRESULT DeviceList::GetDeviceName(UINT32 index, WCHAR **ppszName)
{
    // make sure no clear can be called after we checked the index.
    QMutexLocker lock(&m_enumerationMutex);
    if (index >= Count())
    {
        return E_INVALIDARG;
    }

    *ppszName = new wchar_t[deviceNames[index].size()+1];
    wcscpy (*ppszName, deviceNames[index].toStdWString().c_str());

    HRESULT hr = S_OK;

    /*
    UINT32 cchName;

    hr = m_ppDevices[index]->GetAllocatedString(
                MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                ppszName,
                &cchName
                );
    */
    return hr;
}
