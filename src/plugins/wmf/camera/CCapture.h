#ifndef CCAPTURE_H
#define CCAPTURE_H

#include <QObject>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <Dbt.h>
#include <QVideoFrame>

const UINT WM_APP_PREVIEW_ERROR = WM_APP + 1;    // wparam = HRESULT


class CCapture : public QObject, public IMFSourceReaderCallback
{
    Q_OBJECT
public:
    static HRESULT CreateInstance(CCapture **ppPlayer, int devId
    );

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFSourceReaderCallback methods
    STDMETHODIMP OnReadSample(
        HRESULT hrStatus,
        DWORD dwStreamIndex,
        DWORD dwStreamFlags,
        LONGLONG llTimestamp,
        IMFSample *pSample
    );

    STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *)
    {
        return S_OK;
    }

    STDMETHODIMP OnFlush(DWORD)
    {
        return S_OK;
    }

    HRESULT     StartCapture(IMFActivate *pActivate, unsigned long formatIdx = 0);
    HRESULT     EndCaptureSession();
    BOOL        IsCapturing();
    HRESULT     CheckDeviceLost(DEV_BROADCAST_HDR *pHdr, BOOL *pbDeviceLost);

    HRESULT SetDeviceFormat(IMFMediaSource *pSource, DWORD dwFormatIndex);
    signals:
    void newFrame(int devId, QVideoFrame frame);
protected:

    enum State
    {
        State_NotReady = 0,
        State_Ready,
        State_Capturing,
    };

    // Constructor is private. Use static CreateInstance method to instantiate.
    CCapture(int devId);

    // Destructor is private. Caller should call Release.
    virtual ~CCapture();

    HRESULT OpenMediaSource(IMFMediaSource *pSource);
    HRESULT ConfigureCapture();
    HRESULT EndCaptureInternal();

    int                     m_devId;
    long                    m_nRefCount;        // Reference count.
    CRITICAL_SECTION        m_critsec;

    IMFSourceReader         *m_pReader;

    BOOL                    m_bFirstSample;
    LONGLONG                m_llBaseTime;

    WCHAR                   *m_pwszSymbolicLink;


    // from ESCAPI
    HRESULT ConfigureSourceReader(IMFSourceReader *pReader);
    LONG                    mDefaultStride;

    unsigned int			*mCaptureBuffer;
    unsigned int			*mTargetBuffer;
    unsigned int			mCaptureBufferWidth, mCaptureBufferHeight;
};

#endif // CCAPTURE_H
