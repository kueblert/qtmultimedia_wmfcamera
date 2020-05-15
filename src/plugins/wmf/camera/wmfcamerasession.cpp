/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/qdebug.h>
#include <QCoreApplication>
#include <QFile>
#include <QSettings>
#include <QtConcurrent/QtConcurrentRun>
#include <QtMultimedia/qabstractvideobuffer.h>
#include <QtMultimedia/qvideosurfaceformat.h>
#include <QtMultimedia/qcameraimagecapture.h>
#include <private/qmemoryvideobuffer_p.h>
#include <private/qvideoframe_p.h>

#include "wmfcamerasession.h"
#include <Dbt.h>
#include <ks.h>
#include <shlwapi.h>


QT_BEGIN_NAMESPACE

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

WMFCameraSession::WMFCameraSession(QObject *parent)
    : QObject(parent)
    , IMFSourceReaderCallback()
    , m_status(QCamera::UnloadedStatus)
    , nSuccessivePresentationFailures(0)
    , maxSuccessivePresentationFailures(5)
    , m_pwszSymbolicLink(NULL)
    , hwts_usec(-1)
    , IMFSource(nullptr)
    , ActivateSource(nullptr),
      referenceCount(1)
{
    m_deviceLostEventTimer.setSingleShot(true);

    //qInfo() << "WMFCameraSession";
    // Initialize the COM library
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    // Initialize Media Foundation
    if (SUCCEEDED(hr))
    {
        hr = MFStartup(MF_VERSION);
    }

    g_hdevnotify = NULL;
    // Register for device notifications
    if (SUCCEEDED(hr))
    {
        DEV_BROADCAST_DEVICEINTERFACE di = { 0 };

        di.dbcc_size = sizeof(di);
        di.dbcc_devicetype  = DBT_DEVTYP_DEVICEINTERFACE;
        di.dbcc_classguid  = KSCATEGORY_CAPTURE;

        g_hdevnotify = RegisterDeviceNotification(
                    NULL,
                    &di,
                    DEVICE_NOTIFY_WINDOW_HANDLE
                    );

        if (g_hdevnotify == NULL)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            qWarning() << "MF initialization error.";
        }
        if(FAILED(hr)){
            qWarning() << "Retrieving error failed";
        }
    }
    else{
        qWarning() << "MF initialization failed.";
    }
    //qInfo() << "WMFCameraSession done.";
}

WMFCameraSession::~WMFCameraSession()
{
    unload();
    MFShutdown();
    CoUninitialize();

    if (g_hdevnotify)
    {
        UnregisterDeviceNotification(g_hdevnotify);
    }

}

void WMFCameraSession::setSurface(QAbstractVideoSurface* surface)
{
    //qInfo() << "setSurface";
    QMutexLocker locker(&surfaceMutex);
    m_surface = surface;
}

bool WMFCameraSession::setDevice(const QString &device)
{
    //qInfo() << "setDevice";
    deviceName = device;
    devicelist.EnumerateDevices();
    WCHAR *szFriendlyName = NULL;

    for (UINT32 iDevice = 0; iDevice < devicelist.Count(); iDevice++)
    {
        // Get the friendly name of the device.
        HRESULT hr = devicelist.GetDeviceName(iDevice, &szFriendlyName);
        if(device == QString::fromWCharArray(szFriendlyName)){
            IMFActivate *pActivate = NULL;
            hr = devicelist.GetDevice(1, &pActivate);

        }
        //qInfo() << "device identified";

        CoTaskMemFree(szFriendlyName);
        hr = devicelist.GetDevice(1, &ActivateSource);


        if (SUCCEEDED(hr))
        {
            //qInfo() << "device active";
            return true;
        }
        else{
            qWarning() << "Error in setDevice: ActivateObject";
            return false;
        }
    }
    qWarning() << "Requested device not found.";
    return false;
}


bool WMFCameraSession::load()
{
    unload();
    assert(ActivateSource);

    qInfo() << "ActivateObject " << deviceName;
    // Create the media source for the device.
    HRESULT hr = ActivateSource->ActivateObject(
                __uuidof(IMFMediaSource),
                (void**)&IMFSource
                );

    if (SUCCEEDED(hr))
    {
        updateSourceCapabilities();
        return true;
    }
    else{
        qWarning() << "Error in load: ActivateObject";
        return false;
    }

}

bool WMFCameraSession::unload()
{
    //TODO
    qInfo() << "unload";

    //SafeRelease(&m_pReader);
    if(IMFSource){
        IMFSource->Shutdown();
    }
    SafeRelease(&IMFSource);


    m_supportedViewfinderSettings.clear();

    return true;
}


bool WMFCameraSession::startPreview()
{
    //qInfo() << "startPreview";
    return SUCCEEDED(StartCapture(ActivateSource));
}

bool WMFCameraSession::stopPreview(){
    //qInfo() << "stopPreview";
    return SUCCEEDED(EndCaptureSession());
}


QCameraViewfinderSettings WMFCameraSession::viewfinderSettings() const
{
    return m_viewfinderSettings;
}

void WMFCameraSession::setViewfinderSettings(const QCameraViewfinderSettings &settings)
{
    m_viewfinderSettings = settings;
    interval = 1 / settings.maximumFrameRate();
    //qInfo() << "setViewfinderSettings " << m_viewfinderSettings.resolution() << " " << m_viewfinderSettings.pixelFormat() << " " << m_viewfinderSettings.maximumFrameRate();

}


bool WMFCameraSession::isImageProcessingParameterSupported(
        QCameraImageProcessingControl::ProcessingParameter parameter) const
{
    // TODO
    Q_UNUSED(parameter);
    return false;
}

bool WMFCameraSession::isImageProcessingParameterValueSupported(
        QCameraImageProcessingControl::ProcessingParameter parameter,
        const QVariant &value) const
{    // TODO
    Q_UNUSED(parameter);
    Q_UNUSED(value);
    return false;
}

QVariant WMFCameraSession::imageProcessingParameter(
        QCameraImageProcessingControl::ProcessingParameter parameter) const
{
    /*
            IAMVideoProcAmp *pProcAmp = NULL;
            HRESULT hr = IMFSource->QueryInterface(IID_PPV_ARGS(&pProcAmp));
            if (SUCCEEDED(hr))
             {
                long lMin, lMax, lStep, lDefault, lCaps;
                hr = pProcAmp->GetRange(
                                  VideoProcAmp_Brightness,
                                  &lMin,
                                  &lMax,
                                  &lStep,
                                  &lDefault,
                                  &lCaps
                );
            }
            */
    //TODO

    switch (parameter) {
    /*case QCameraImageProcessingControl::BrightnessAdjustment:
        return uvc_get(devh, uvc_get_brightness);
    case QCameraImageProcessingControl::ContrastAdjustment:
        return uvc_get(devh, uvc_get_contrast);
    case QCameraImageProcessingControl::ColorTemperature:
        return uvc_get(devh, uvc_get_white_balance_temperature);
    case QCameraImageProcessingControl::SaturationAdjustment:
        return uvc_get(devh, uvc_get_saturation);
    case QCameraImageProcessingControl::SharpeningAdjustment:
        return uvc_get(devh, uvc_get_sharpness);*/
    default:
        qWarning() << parameter << "not supported";
        return QVariant();
    }
}

void WMFCameraSession::setImageProcessingParameter(
        QCameraImageProcessingControl::ProcessingParameter parameter,
        const QVariant &value)
{
    /*
    IAMVideoProcAmp *pProcAmp = NULL;
    HRESULT hr = IMFSource->QueryInterface(IID_PPV_ARGS(&pProcAmp));

    if (SUCCEEDED(hr))
        {
           hr = pProcAmp->Set(
                           VideoProcAmp_Brightness,
                           value.toInt(),
                           VideoProcAmp_Flags_Manual
           );
        }
    */

    switch (parameter) {
    /*case QCameraImageProcessingControl::BrightnessAdjustment:
        uvc_set(devh, uvc_set_brightness, value, uvc_get_brightness);
        break;
    case QCameraImageProcessingControl::ContrastAdjustment:
        uvc_set(devh, uvc_set_contrast, value, uvc_get_contrast);
        break;
    case QCameraImageProcessingControl::ColorTemperature:
        uvc_set(devh, uvc_set_white_balance_temperature, value,
                uvc_get_white_balance_temperature);
        break;
    case QCameraImageProcessingControl::SaturationAdjustment:
        uvc_set(devh, uvc_set_saturation, value, uvc_get_saturation);
        break;
    case QCameraImageProcessingControl::SharpeningAdjustment:
        uvc_set(devh, uvc_set_sharpness, value, uvc_get_sharpness);
        break;*/
    default:
        qWarning() << parameter << "not supported";
    }
}


void WMFCameraSession::presentFrame(const QVideoFrame& frame)
{
    //qInfo() << "presentFrame " << frame.isValid();

    // If no frames provided from ISampleGrabber for some time
    // the device might be potentially unplugged.
    //m_deviceLostEventTimer.start(100);
    if (m_surface){
        // this one slows everything down remarkably.
        bool ok = m_surface->present(frame);
        if(!ok){
            nSuccessivePresentationFailures++;
            if(nSuccessivePresentationFailures > maxSuccessivePresentationFailures){
                qInfo() << "Restarting camera session due to " << nSuccessivePresentationFailures << " consecutive frame presentation errors.";
                nSuccessivePresentationFailures = 0;
                qDebug() << "stopPreview" << stopPreview();
                qDebug() << "unload: " << unload();
                qDebug() << "load: " << load();
                qDebug() << "startPreview" << startPreview();
            }
        }
        else{
            nSuccessivePresentationFailures = 0;
        }
    }
    else{
        qWarning() << "QAbstractVideoSurface not ready";
    }
    //qInfo() << "presentFrame done.";
}

/////////////// IMFSourceReaderCallback methods ///////////////

//-------------------------------------------------------------------
// OnReadSample
//
// Called when the IMFMediaSource::ReadSample method completes.
//-------------------------------------------------------------------

HRESULT WMFCameraSession::OnReadSample(
        HRESULT hrStatus,
        DWORD /*dwStreamIndex*/,
        DWORD /*dwStreamFlags*/,
        LONGLONG llTimeStamp,
        IMFSample *pSample      // Can be NULL
        )
{
    //qInfo() << "OnReadSample";
    auto now = std::chrono::steady_clock::now();
    //EnterCriticalSection(&m_critsec);

    if (!IsCapturing())
    {
        qInfo() << "!IsCapturing";
        //LeaveCriticalSection(&m_critsec);
        return S_OK;
    }


    QMutexLocker locker(&surfaceMutex);

    HRESULT hr = S_OK;

    if (FAILED(hrStatus))
    {
        hr = hrStatus;
        qInfo() << deviceName << ": hrStatus failed - ";
        goto done;
    }

    //qInfo() << "check sample";
    if (pSample)
    {
        //qInfo() << "sample not null";
        if (m_bFirstSample)
        {
            //qInfo() << "First Sample";
            m_llBaseTime = llTimeStamp;
            m_bFirstSample = FALSE;
        }

        // rebase the time stamp
        llTimeStamp -= m_llBaseTime;

        hr = pSample->SetSampleTime(llTimeStamp);

        if (FAILED(hr)) { goto done; }
    }

    // Read another sample.
    hr = m_pReader->ReadSample(
                (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                0,
                NULL,   // actual
                NULL,   // flags
                NULL,   // timestamp
                NULL    // sample
                );

done:
    //qDebug() << "Reading done";
    if (FAILED(hr))
    {
        qWarning() << "Error reading sample:";
        std::string message = std::system_category().message(hr);
        qWarning() << QString::fromStdString(message) << "/" << hr;
    }

    //LeaveCriticalSection(&m_critsec);

    if(pSample){
        //qInfo() << "Copy buffer";
        // here we copy the actual data
        IMFMediaBuffer* buffer;
        pSample->ConvertToContiguousBuffer(&buffer);
        //qInfo() << "ConvertToContiguousBuffer";
        BYTE* data;
        DWORD capacity;
        DWORD length;
        //IMF2DBuffer* imbuffer;
        //buffer->QueryInterface(IID_IMF2DBuffer, (void**)&imbuffer);

        buffer->Lock(&data, &capacity, &length);
        //qInfo() << "Lock";
        QVideoFrame qFrame(length,
                           m_viewfinderSettings.resolution(), 0,
                           QVideoFrame::Format_Jpeg);
        qFrame.map(QAbstractVideoBuffer::WriteOnly);
        //qInfo() << "map";
        memcpy(qFrame.bits(), data,
               length); // copied; safe to retun from callback now
        //qInfo() << "memcpy";
        qFrame.unmap();
        buffer->Unlock();
        SafeRelease(&buffer);
        //qInfo() << "Buffer released";
        //TODO
        //qFrame.setMetaData("steady_clock::now", QVariant::fromValue(now));
        qFrame.setStartTime(llTimeStamp);

        qFrame.setStartTime(hwts_usec);
        hwts_usec = llTimeStamp;
        qFrame.setEndTime(hwts_usec);

        //qInfo() << "invoke present";
        QMetaObject::invokeMethod(this, "presentFrame", Qt::QueuedConnection, Q_ARG(QVideoFrame, qFrame));
    }
    //qInfo() << "OnRead finished";
    return hr;
}



//-------------------------------------------------------------------
// OpenMediaSource
//
// Set up preview for a specified media source.
//-------------------------------------------------------------------

HRESULT WMFCameraSession::OpenMediaSource(IMFMediaSource *pSource)
{
    //qInfo() << "OpenMediaSource";
    HRESULT hr = S_OK;

    IMFAttributes *pAttributes = NULL;

    hr = MFCreateAttributes(&pAttributes, 2);

    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);
    }
    else{
        qWarning() << "MFCreateAttributes failed";
    }

    if (SUCCEEDED(hr))
    {
        qInfo() << "MFCreateSourceReaderFromMediaSource " << deviceName;
        hr = MFCreateSourceReaderFromMediaSource(
                    pSource,
                    pAttributes,
                    &m_pReader
                    );
    }
    else{
        qWarning() << "SetUnknown failed";
    }
    if (FAILED(hr)){
        qWarning() << "MFCreateSourceReaderFromMediaSource failed";
    }

    SafeRelease(&pAttributes);
    return hr;
}

HRESULT WMFCameraSession::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(WMFCameraSession, IMFSourceReaderCallback),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

HRESULT WMFCameraSession::StartCapture(
        IMFActivate *pActivate
        )
{
    assert(pActivate != nullptr);
    assert(IMFSource != nullptr);

    qInfo() << "StartCapture";
    HRESULT hr = S_OK;
    //EnterCriticalSection(&m_critsec);
    //qInfo() << "EnterCriticalSection";
    // should be active already after load()
    IMFMediaSource *pSource = IMFSource;
    qInfo() << "setMediaFormat call";
    setMediaFormat();
    /*
    //qInfo() << "activate";
    // Create the media source for the device.
    hr = pActivate->ActivateObject(
                __uuidof(IMFMediaSource),
                (void**)&pSource
                );
*/
    // Get the symbolic link. This is needed to handle device-
    // loss notifications. (See CheckDeviceLost.)

    if (SUCCEEDED(hr))
    {
        qInfo() << "symbolic link";
        UINT32 cchName;
        hr = pActivate->GetAllocatedString(
                    MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
                    &m_pwszSymbolicLink,
                    &cchName
                    );

        qInfo() << "symbolic link done";
    }
    else{
        qWarning() << "Error ActivateObject";


        std::string message = std::system_category().message(hr);
        qDebug() << QString::fromStdString(message);
    }

    if (SUCCEEDED(hr))
    {
        hr = OpenMediaSource(pSource);
    }
    else{
        qWarning() << "Error GetAllocatedString";


        //std::string message = std::system_category().message(hr);
        //qDebug() << QString::fromStdString(message);
    }


    // Set up the encoding parameters.

    if (SUCCEEDED(hr))
    {
        //qInfo() << "ConfigureSourceReader";
        IMFMediaType *pType = NULL;

        //hr = ConfigureSourceReader(m_pReader);
        /*
        if (SUCCEEDED(hr))
        {
            //qInfo() << "GetCurrentMediaType";
            hr = m_pReader->GetCurrentMediaType(
                        (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                        &pType
                        );
        }
        */
        SafeRelease(&pType);
    }
    else{
        qWarning() << "Error MFCreateSinkWriterFromURL";


        std::string message = std::system_category().message(hr);
        qDebug() << QString::fromStdString(message);
    }



    if (SUCCEEDED(hr))
    {
        m_bFirstSample = TRUE;
        m_llBaseTime = 0;

        // Request the first video frame.

        qInfo() << "ReadSample init";

        hr = m_pReader->ReadSample(
                    (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                    0,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );
    }
    else{
        qWarning() << "OpenMediaSource failed" << hr;
    }

    if(FAILED(hr)){
        qWarning() << "ReadSample failed" << hr;
        std::string message = std::system_category().message(hr);
        qDebug() << QString::fromStdString(message);
    }

    SafeRelease(&pSource);
    //LeaveCriticalSection(&m_critsec);
    qDebug() << "Configured";
    return hr;
}


HRESULT WMFCameraSession::EndCaptureSession()
{
    //EnterCriticalSection(&m_critsec);

    HRESULT hr = S_OK;

    SafeRelease(&m_pReader);

    //LeaveCriticalSection(&m_critsec);

    return hr;
}

BOOL WMFCameraSession::IsCapturing()
{
    //EnterCriticalSection(&m_critsec);

    BOOL bIsCapturing = (m_pReader != NULL);

    //LeaveCriticalSection(&m_critsec);

    return bIsCapturing;
}


//-------------------------------------------------------------------
//  ConfigureSourceReader
//
//  Sets the media type on the source reader.
//-------------------------------------------------------------------

HRESULT WMFCameraSession::ConfigureSourceReader(IMFSourceReader *pReader)
{
    // The list of acceptable types.
    GUID subtypes[] = {
        MFVideoFormat_NV12, MFVideoFormat_YUY2, MFVideoFormat_UYVY,
        MFVideoFormat_RGB32, MFVideoFormat_RGB24, MFVideoFormat_IYUV
    };

    HRESULT hr = S_OK;
    BOOL    bUseNativeType = FALSE;

    GUID subtype = MFVideoFormat_RGB32;

    IMFMediaType *pType = NULL;

    // If the source's native format matches any of the formats in
    // the list, prefer the native format.

    // Note: The camera might support multiple output formats,
    // including a range of frame dimensions. The application could
    // provide a list to the user and have the user select the
    // camera's output format. That is outside the scope of this
    // sample, however.

    //select the stream that should be used.
    hr = pReader->GetNativeMediaType(
                (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                0,  // Type index
                &pType
                );

    if (FAILED(hr)) { goto done; }

    hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);

    if (FAILED(hr)) { goto done; }

    for (UINT32 i = 0; i < ARRAYSIZE(subtypes); i++)
    {
        if (subtype == subtypes[i])
        {
            hr = pReader->SetCurrentMediaType(
                        (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                        NULL,
                        pType
                        );

            bUseNativeType = TRUE;
            break;
        }
    }

    if (!bUseNativeType)
    {
        // None of the native types worked. The camera might offer
        // output a compressed type such as MJPEG or DV.

        // Try adding a decoder.

        for (UINT32 i = 0; i < ARRAYSIZE(subtypes); i++)
        {
            hr = pType->SetGUID(MF_MT_SUBTYPE, subtypes[i]);

            if (FAILED(hr)) { goto done; }

            hr = pReader->SetCurrentMediaType(
                        (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                        NULL,
                        pType
                        );

            if (SUCCEEDED(hr))
            {
                break;
            }
        }
    }

done:
    SafeRelease(&pType);
    return hr;
}

void WMFCameraSession::updateSourceCapabilities(){
    m_supportedViewfinderSettings.clear();

    assert(ActivateSource);
    assert(IMFSource);

    IMFPresentationDescriptor *pPD = NULL;
    IMFStreamDescriptor *pSD = NULL;
    IMFMediaTypeHandler *pHandler = NULL;
    IMFMediaType *pType = NULL;


    HRESULT hr = IMFSource->CreatePresentationDescriptor(&pPD);
    if (FAILED(hr))
    {
        goto done;
    }

    DWORD streamCount;
    pPD->GetStreamDescriptorCount(&streamCount);
    for(uint j=0; j < streamCount; j++){


        BOOL fSelected;
        hr = pPD->GetStreamDescriptorByIndex(j, &fSelected, &pSD);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = pSD->GetMediaTypeHandler(&pHandler);
        if (FAILED(hr))
        {
            goto done;
        }

        DWORD cTypes = 0;
        hr = pHandler->GetMediaTypeCount(&cTypes);
        if (FAILED(hr))
        {
            goto done;
        }

        for (DWORD i = 0; i < cTypes; i++)
        {
            hr = pHandler->GetMediaTypeByIndex(i, &pType);
            if (FAILED(hr))
            {
                goto done;
            }

            // construct the QVideoFormat here.
            QString formatStr;

            // we do not query this from the camera, which sucks but it is relatively hard ;-)
            QVideoFrame::PixelFormat qFormat;

            // query parameters
            UINT32 pdwWidthInPixels, pdwHeightInPixels;
            MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &pdwWidthInPixels, &pdwHeightInPixels);
            UINT32 pnNumerator, pnDenominator;
            MFGetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, &pnNumerator, &pnDenominator);
            UINT32 pnNumeratorFPS, pnDenominatorFPS;
            MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &pnNumeratorFPS, &pnDenominatorFPS);

            DWORD pFourCC;
            GUID guidSubType = GUID_NULL;
            pType->GetGUID(MF_MT_SUBTYPE, &guidSubType);
            pFourCC = guidSubType.Data1;


            QString fourCC = QString::fromUtf8(reinterpret_cast<char const *>(&pFourCC));
            fourCC = fourCC.left(4);

            if(pFourCC == FCC('MJPG')){
                qFormat = QVideoFrame::Format_Jpeg;
            }
            if(pFourCC == FCC('NV12')){
                qFormat = QVideoFrame::Format_NV12;
            }

            QCameraViewfinderSettings settings;
            settings.setResolution(pdwWidthInPixels, pdwHeightInPixels);
            settings.setPixelAspectRatio(pnNumerator, pnDenominator);

            settings.setPixelFormat(qFormat);
            double fps = ((double)pnNumeratorFPS) / ((double)pnDenominatorFPS);

            qDebug() << fourCC << " (" << pdwWidthInPixels << "x" << pdwHeightInPixels << ") @ " << fps << " fps";

            settings.setMinimumFrameRate(fps);
            settings.setMaximumFrameRate(fps);
            m_supportedViewfinderSettings.push_back(settings);

            SafeRelease(&pType);
        }
        SafeRelease(&pSD);
        SafeRelease(&pHandler);
    }
done:
    SafeRelease(&pPD);
    SafeRelease(&pSD);
    SafeRelease(&pHandler);
    SafeRelease(&pType);

    if(FAILED(hr)){
        qWarning() << "An error occurred while querying stram formats";
    }
}


void WMFCameraSession::setMediaFormat(){

    assert(ActivateSource);
    assert(IMFSource);

    IMFPresentationDescriptor *pPD = NULL;
    IMFStreamDescriptor *pSD = NULL;
    IMFMediaTypeHandler *pHandler = NULL;
    IMFMediaType *pType = NULL;

    //qInfo() << "setMediaFormat " << m_viewfinderSettings.resolution() << " " << m_viewfinderSettings.pixelFormat() << " " << m_viewfinderSettings.maximumFrameRate();

    HRESULT hr = IMFSource->CreatePresentationDescriptor(&pPD);
    if (FAILED(hr))
    {
        goto done;
    }

    DWORD streamCount;
    pPD->GetStreamDescriptorCount(&streamCount);
    for(uint j=0; j < streamCount; j++){


        BOOL fSelected;
        hr = pPD->GetStreamDescriptorByIndex(j, &fSelected, &pSD);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = pSD->GetMediaTypeHandler(&pHandler);
        if (FAILED(hr))
        {
            goto done;
        }

        DWORD cTypes = 0;
        hr = pHandler->GetMediaTypeCount(&cTypes);
        if (FAILED(hr))
        {
            goto done;
        }

        for (DWORD i = 0; i < cTypes; i++)
        {
            hr = pHandler->GetMediaTypeByIndex(i, &pType);
            if (FAILED(hr))
            {
                goto done;
            }

            // construct the QVideoFormat here.
            QString formatStr;

            // we do not query this from the camera, which sucks but it is relatively hard ;-)
            QVideoFrame::PixelFormat qFormat;

            // query parameters
            UINT32 pdwWidthInPixels, pdwHeightInPixels;
            MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &pdwWidthInPixels, &pdwHeightInPixels);
            UINT32 pnNumerator, pnDenominator;
            MFGetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, &pnNumerator, &pnDenominator);
            UINT32 pnNumeratorFPS, pnDenominatorFPS;
            MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &pnNumeratorFPS, &pnDenominatorFPS);


            DWORD pFourCC;
            GUID guidSubType = GUID_NULL;
            pType->GetGUID(MF_MT_SUBTYPE, &guidSubType);
            pFourCC = guidSubType.Data1;


            QString fourCC = QString::fromUtf8(reinterpret_cast<char const *>(&pFourCC));
            fourCC = fourCC.left(4);
            if(pFourCC == FCC('MJPG')){
                qFormat = QVideoFrame::Format_Jpeg;
            }
            if(pFourCC == FCC('NV12')){
                qFormat = QVideoFrame::Format_NV12;
            }

            QCameraViewfinderSettings settings;
            settings.setResolution(pdwWidthInPixels, pdwHeightInPixels);
            settings.setPixelAspectRatio(pnNumerator, pnDenominator);


            settings.setPixelFormat(qFormat);
            double fps = ((double)pnNumeratorFPS) / ((double)pnDenominatorFPS);
            settings.setMinimumFrameRate(fps);
            settings.setMaximumFrameRate(fps);

            // is this the required stream that matches the query?
            if(m_viewfinderSettings.resolution().width() == pdwWidthInPixels
                    && m_viewfinderSettings.resolution().height() == pdwHeightInPixels
                    && m_viewfinderSettings.minimumFrameRate() == fps
                    && m_viewfinderSettings.maximumFrameRate() == fps
                    && m_viewfinderSettings.pixelFormat() == qFormat
                    ){
                //qInfo() << "Format match found!";
                hr = pHandler->SetCurrentMediaType(pType);
                if(FAILED(hr)){
                    qWarning() << "Format could not be set!";
                }
                goto done;
            }

            SafeRelease(&pType);
        }
        SafeRelease(&pSD);
        SafeRelease(&pHandler);
    }
done:
    SafeRelease(&pPD);
    SafeRelease(&pSD);
    SafeRelease(&pHandler);
    SafeRelease(&pType);

    if(FAILED(hr)){
        qWarning() << "An error occurred while querying stream formats";
    }


}


QT_END_NAMESPACE
