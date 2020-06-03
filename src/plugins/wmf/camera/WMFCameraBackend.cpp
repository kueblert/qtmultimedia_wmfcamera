#include "WMFCameraBackend.h"
#include <QDebug>
#include <chrono>
#include <Windows.h>

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

WMFCameraBackend::WMFCameraBackend():
    QObject()
{
    gParams = new QMap<int, SimpleCapParams>();
}

WMFCameraBackend::~WMFCameraBackend(){
    CoUninitialize();
    delete gParams;
}

bool WMFCameraBackend::initApp(){
    HRESULT hr = S_OK;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if(FAILED(hr)){
        qWarning() << "Failed to CoInitializeEx";
        return false;
    }
    hr = MFStartup(MF_VERSION);
    if (FAILED(hr)){
        qWarning() << "Failed to initialize Media Foundation";
    }
    return SUCCEEDED(hr);
}

void WMFCameraBackend::init(){

    if(!initApp())
        qWarning() << "Unable to initialize";

    dl.Clear();
    dl.EnumerateDevices();

}

void WMFCameraBackend::setResolution(int devId, int width, int height, const QVideoFrame::PixelFormat& format){
    qWarning() << format;
    SimpleCapParams p;
    p.mWidth = width; p.mHeight = height;
    switch(format){
    case QVideoFrame::Format_Jpeg:
        p.mFormat= MFVideoFormat_MJPG; break;
    case QVideoFrame::Format_NV12:
        p.mFormat = MFVideoFormat_NV12; break;
    default:
        qWarning() << "Requested pixel format " << format << " was not converted. Defaulting to MJPG.";
        p.mFormat = MFVideoFormat_MJPG;
    }

    setProperties(devId, p);
}

void WMFCameraBackend::setProperties(int devId, SimpleCapParams param){

    // handles a change even if ddevice is already running. The device is restarted for this purpose.
    bool isRunning = m_runningDevices.contains(devId);
    if(isRunning){
        freeDevice(devId);
    }

    gParams->operator[](devId) = param;

    if(isRunning){
        requestDevice(devId);
    }
}

void WMFCameraBackend::CheckForFail(int devId)
{
    if (!m_runningDevices[devId])
        return;

    if (m_runningDevices[devId]->mRedoFromStart)
    {
        m_runningDevices[devId]->mRedoFromStart = 0;
        m_runningDevices[devId]->deinitCapture();
        HRESULT hr = m_runningDevices[devId]->initCapture(devId);
        if (FAILED(hr))
        {
            delete m_runningDevices[devId];
            m_runningDevices[devId] = 0;
        }
    }
}

float WMFCameraBackend::GetProperty(int aDevice, int aProp)
{
    CheckForFail(aDevice);
    if (!m_runningDevices[aDevice])
        return 0;
    float val;
    int autoval;
    m_runningDevices[aDevice]->getProperty(aProp, val, autoval);
    return val;
}

int WMFCameraBackend::GetPropertyAuto(int aDevice, int aProp)
{
    CheckForFail(aDevice);
    if (!m_runningDevices[aDevice])
        return 0;
    float val;
    int autoval;
    m_runningDevices[aDevice]->getProperty(aProp, val, autoval);
    return autoval;
}

int WMFCameraBackend::SetProperty(int aDevice, int aProp, float aValue, int aAutoval)
{
    CheckForFail(aDevice);
    if (!m_runningDevices[aDevice])
        return 0;
    return m_runningDevices[aDevice]->setProperty(aProp, aValue, aAutoval);
}

void WMFCameraBackend::requestDevice(int devId){

    if(!gParams->contains(devId)){
        qWarning() << "No format specified! Defaulting.";
        // choose a default
        SimpleCapParams p;
        p.mWidth = 320; p.mHeight = 240;
        p.mFormat = MFVideoFormat_MJPG;
        gParams->operator[](devId) = p;
    }

    if(m_runningDevices.contains(devId)){
        freeDevice(devId);
    }

    gParams->operator[](devId).mBufferBytes = gParams->operator[](devId).mWidth*gParams->operator[](devId).mHeight;
    gParams->operator[](devId).mTargetBuf = new unsigned int[gParams->operator[](devId).mBufferBytes];
    gParams->operator[](devId).doCapture = true;
    gOptions[devId] = 0;

    CaptureClass* capture = new CaptureClass(gParams);
    m_runningDevices[devId] = capture;
    if(FAILED(capture->initCapture(devId))){
        m_runningDevices.remove(devId);
        delete capture;
    }
    else{
        connect(capture, &CaptureClass::newFrame, this, &WMFCameraBackend::newFrameReceived);
    }
}

void WMFCameraBackend::freeDevice(int devId){
    if(m_runningDevices.contains(devId)){
        disconnect(m_runningDevices[devId], &CaptureClass::newFrame, this, &WMFCameraBackend::newFrameReceived);
        m_runningDevices[devId]->deinitCapture();
        m_runningDevices.remove(devId);
        if(gParams->operator[](devId).mTargetBuf){
            delete gParams->operator[](devId).mTargetBuf;
            gParams->operator[](devId).mTargetBuf = NULL;
            gParams->operator[](devId).mBufferBytes = 0;
        }
    }
    else{
        qWarning() << "Internal error freeing non-registered device id.";
    }
}

QList<QByteArray> WMFCameraBackend::getDeviceNames(){
    QMutexLocker locker(&m_devicePorps);

    dl.EnumerateDevices();
    QList<QByteArray> avail_devices;
    WCHAR *szFriendlyName = NULL;
    for (UINT32 iDevice = 0; iDevice < dl.Count(); iDevice++)
    {
        dl.GetDeviceName(iDevice, &szFriendlyName);
        avail_devices.append(QString::fromWCharArray(szFriendlyName).toUtf8());
    }
    return avail_devices;
}

void WMFCameraBackend::newFrameReceived(int devId){
    assert(gParams->contains(devId));
    unsigned int* rawFrame = gParams->operator[](devId).mTargetBuf;
    //qDebug() << "constructing frame: " << gParams->operator[](devId).mWidth << "x" << gParams->operator[](devId).mHeight << " bytes: " << gParams->operator[](devId).mBufferBytes;
    int bytes = sizeof(rawFrame) * gParams->operator[](devId).mWidth*gParams->operator[](devId).mHeight;
    assert(m_runningDevices.contains(devId));

    QVideoFrame::PixelFormat format = gParams->operator[](devId).mFormat == MFVideoFormat_MJPG?QVideoFrame::Format_Jpeg : QVideoFrame::Format_NV12;
    //TODO where does that 4 come from?!
    QVideoFrame qFrame = QVideoFrame(
                gParams->operator[](devId).mBufferBytes*4, //sizeof(rawFrame),
                QSize(gParams->operator[](devId).mWidth,gParams->operator[](devId).mHeight),
                sizeof(rawFrame) * gParams->operator[](devId).mWidth,
                format);
    bool mapOk = qFrame.map(QAbstractVideoBuffer::WriteOnly);
    if(!mapOk){
        qWarning() << "Map failed!";
    }
    //qDebug() << "Copying " << pixels << " pixels a " << sizeof(capture[k].mTargetBuf) << "bytes";
    //memcpy(qFrame.bits(), gParams->operator[](devId).mTargetBuf, bytes); // copied; safe to retun from callback now
    memcpy(qFrame.bits(), gParams->operator[](devId).mTargetBuf, gParams->operator[](devId).mBufferBytes*4); // copied; safe to return from callback now
    qFrame.unmap();
    gParams->operator[](devId).doCapture = true; // allow the capture to record more data

    //qDebug() << "Constructed frame " << qFrame;
    emit newFrame(devId, qFrame);
}
