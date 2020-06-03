#pragma once
#include <QObject>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include "conversion.h"
#include "escapi_minimal.h"

#include <shlwapi.h> // QITAB and friends
#include <objbase.h> // IID_PPV_ARGS and friends
#include <dshow.h> // IAMVideoProcAmp and friends


class CaptureClass : public QObject, public IMFSourceReaderCallback
{
    Q_OBJECT
public:

    CaptureClass(QMap<int, SimpleCapParams>* params);
	~CaptureClass();
	STDMETHODIMP QueryInterface(REFIID aRiid, void** aPpv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP OnReadSample(
		HRESULT aStatus,
		DWORD aStreamIndex,
		DWORD aStreamFlags,
		LONGLONG aTimestamp,
		IMFSample *aSample
		);
	STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *);
	STDMETHODIMP OnFlush(DWORD);
	int escapiPropToMFProp(int aProperty);
	int setProperty(int aProperty, float aValue, int aAuto);
	int getProperty(int aProperty, float &aValue, int &aAuto);
	BOOL isFormatSupported(REFGUID aSubtype) const;
	HRESULT getFormat(DWORD aIndex, GUID *aSubtype) const;
	HRESULT setConversionFunction(REFGUID aSubtype);
	HRESULT setVideoType(IMFMediaType *aType);
	int isMediaOk(IMFMediaType *aType, int aIndex);
	int scanMediaTypes(unsigned int aWidth, unsigned int aHeight);
	HRESULT initCapture(int aDevice);
	void deinitCapture();

	long                    mRefCount;        // Reference count.
	CRITICAL_SECTION        mCritsec;

	IMFSourceReader         *mReader;
	IMFMediaSource			*mSource;

	LONG                    mDefaultStride;
    IMAGE_TRANSFORM_FN      mConvertFn;    // Function to convert the video to RGB32

	unsigned int			*mCaptureBuffer;
	unsigned int			mCaptureBufferWidth, mCaptureBufferHeight;
	int						mErrorLine;
	int						mErrorCode;
	int						mWhoAmI;
	unsigned int			*mBadIndex;
	unsigned int			mBadIndices;
	unsigned int			mMaxBadIndices;
	unsigned int			mUsedIndex;
	int						mRedoFromStart;


    GUID mRequestedFormat;

signals:
    void newFrame(int devId);

private:
    QMap<int, SimpleCapParams>* gParams;
};
