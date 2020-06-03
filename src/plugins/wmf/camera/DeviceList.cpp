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

    SafeRelease(&pAttributes);

    return hr;
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

    HRESULT hr = S_OK;
    UINT32 cchName;
    hr = m_ppDevices[index]->GetAllocatedString(
                MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                ppszName,
                &cchName
                );

    return hr;
}
