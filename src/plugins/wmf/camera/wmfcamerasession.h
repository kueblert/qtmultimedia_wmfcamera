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

#ifndef DSCAMERASESSION_H
#define DSCAMERASESSION_H

#include <QtCore/qobject.h>
#include <QTime>
#include <QUrl>
#include <QMutex>
#include <QTimer>
#include <qcamera.h>
#include <QtMultimedia/qvideoframe.h>
#include <QtMultimedia/qabstractvideosurface.h>
#include <QtMultimedia/qvideosurfaceformat.h>
#include <QtMultimedia/qcameraimageprocessingcontrol.h>
#include <QtMultimedia/qcameraimagecapture.h>
#include <QtMultimedia/qmediaencodersettings.h>
#include <private/qmediastoragelocation_p.h>
#include "DeviceList.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <Wmcodecdsp.h>
#include <chrono>

#pragma comment(lib, "Shlwapi.lib")


#ifdef Q_CC_MSVC
#  pragma comment(lib, "strmiids.lib")
#  pragma comment(lib, "ole32.lib")
#endif // Q_CC_MSVC
#include <windows.h>


QT_BEGIN_NAMESPACE

struct DefaultParameters {
    double bandwidthFactor;
    double brightness;
    double contrast;
    double saturation;
    double white_balance;
    double sharpness;
    double backlight_compensation;
    double gamma;
    double gain;
    double exposure_abs;
    double hue;
    int ae_mode;
    int ae_priority;
    int contrast_auto;
    int focus_auto;
    int white_balance_component_auto;
    int white_balance_temperature_auto;
    int hue_auto;
};

class WMFCameraSession : public QObject, public IMFSourceReaderCallback
{
    Q_OBJECT
public:
    WMFCameraSession(QObject *parent = 0);
    ~WMFCameraSession() override;

    QCamera::Status status() const { return m_status; }
    void setSurface(QAbstractVideoSurface* surface);
    bool setDevice(const QString &device);

    bool load();
    bool unload();
    bool startPreview();
    bool stopPreview();

    HRESULT OnEvent(DWORD dwStreamIndex, IMFMediaEvent *pEvent) override { return 0; }
    HRESULT OnFlush(DWORD dwStreamIndex) override { return 0; }
    ULONG AddRef() override {InterlockedIncrement(&referenceCount); return referenceCount;}
    HRESULT QueryInterface(REFIID riid, void** ppv) override;
    ULONG Release() override { ULONG count = InterlockedDecrement(&referenceCount);
                               if (count == 0)
                                   delete this;
                               // For thread safety
                               return count; }


    QCameraViewfinderSettings viewfinderSettings() const;
    void setViewfinderSettings(const QCameraViewfinderSettings &settings);

    QList<QCameraViewfinderSettings> supportedViewfinderSettings() const
    { return m_supportedViewfinderSettings; }

    bool isImageProcessingParameterSupported(
            QCameraImageProcessingControl::ProcessingParameter) const;

    bool isImageProcessingParameterValueSupported(
            QCameraImageProcessingControl::ProcessingParameter,
            const QVariant &) const;

    QVariant imageProcessingParameter(
            QCameraImageProcessingControl::ProcessingParameter) const;

    void setImageProcessingParameter(
            QCameraImageProcessingControl::ProcessingParameter,
            const QVariant &);

private slots:
    void presentFrame(const QVideoFrame &frame);

private:
    QImageEncoderSettings m_imageEncoderSettings;
    QCameraViewfinderSettings m_viewfinderSettings;
    QList<QCameraViewfinderSettings> m_supportedViewfinderSettings;
    QCamera::Status m_status;
    QTimer m_deviceLostEventTimer;
    QAbstractVideoSurface* m_surface;
    QMutex surfaceMutex;
    int nSuccessivePresentationFailures;
    int maxSuccessivePresentationFailures;
    QString deviceName;
    double interval;
    // frame start and end time in microseconds
    qint64 hwts_usec;

    // the actual internal camera handles
    IMFActivate * ActivateSource;
    IMFMediaSource * IMFSource;

    BOOL                    m_bFirstSample;
    LONGLONG                m_llBaseTime;
    IMFSourceReader         *m_pReader;
    CRITICAL_SECTION        m_critsec;
    WCHAR                   *m_pwszSymbolicLink;

    HRESULT OnReadSample(
            HRESULT hrStatus,
            DWORD /*dwStreamIndex*/,
            DWORD /*dwStreamFlags*/,
            LONGLONG llTimeStamp,
            IMFSample *pSample      // Can be NULL
            ) override;
    HRESULT OpenMediaSource(IMFMediaSource *pSource);
    HRESULT StartCapture(IMFActivate *pActivate);

    HRESULT EndCaptureSession();
    BOOL IsCapturing();
    HRESULT ConfigureSourceReader(IMFSourceReader *pReader);

    void updateSourceCapabilities();
    void setMediaFormat();

    DeviceList devicelist;
    HDEVNOTIFY  g_hdevnotify;

    long referenceCount;

};

QT_END_NAMESPACE


#endif
