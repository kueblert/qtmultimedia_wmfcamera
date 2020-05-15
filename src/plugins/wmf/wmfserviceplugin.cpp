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

#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>
#include <QtCore/QFile>

#include "wmfserviceplugin.h"
#if QT_CONFIG(wmf_player)
#include "mfplayerservice.h"
#endif
#include "mfdecoderservice.h"
#include "wmfcameraservice.h"
#include "WMFVideoDeviceControl.h"

#include <mfapi.h>

namespace
{
static int g_refCount = 0;
void addRefCount()
{
    g_refCount++;
    if (g_refCount == 1) {
        CoInitialize(NULL);
        MFStartup(MF_VERSION);
    }
}

void releaseRefCount()
{
    g_refCount--;
    if (g_refCount == 0) {
        MFShutdown();
        CoUninitialize();
    }
}

}

QMediaService* WMFServicePlugin::create(QString const& key)
{
#if QT_CONFIG(wmf_player)
    if (key == QLatin1String(Q_MEDIASERVICE_MEDIAPLAYER)) {
        addRefCount();
        return new MFPlayerService;
    }
#endif
    if (key == QLatin1String(Q_MEDIASERVICE_AUDIODECODER)) {
        addRefCount();
        return new MFAudioDecoderService;
    }
    if (key == QLatin1String(Q_MEDIASERVICE_CAMERA)) {
        addRefCount();
        return new WMFCameraService;
    }
    //qDebug() << "unsupported key:" << key;
    return 0;
}

void WMFServicePlugin::release(QMediaService *service)
{
    delete service;
    releaseRefCount();
}

QMediaServiceProviderHint::Features WMFServicePlugin::supportedFeatures(
        const QByteArray &service) const
{
#if QT_CONFIG(wmf_player)
    if (service == Q_MEDIASERVICE_MEDIAPLAYER)
        return QMediaServiceProviderHint::StreamPlayback;
    else
#else
    Q_UNUSED(service);
#endif
        return QMediaServiceProviderHint::Features();
}

QByteArray WMFServicePlugin::defaultDevice(const QByteArray &service) const
{
    if (service == Q_MEDIASERVICE_CAMERA) {
        addRefCount();
        const QList<WMFVideoDeviceInfo> &devs = WMFVideoDeviceControl::availableDevices();
        releaseRefCount();
        if (!devs.isEmpty())
            return devs.first().first;
    }
    return QByteArray();
}

QList<QByteArray> WMFServicePlugin::devices(const QByteArray &service) const
{
    QList<QByteArray> result;

    if (service == Q_MEDIASERVICE_CAMERA) {
        addRefCount();
        const QList<WMFVideoDeviceInfo> &devs = WMFVideoDeviceControl::availableDevices();
        releaseRefCount();
        for (const WMFVideoDeviceInfo &info : devs)
            result.append(info.first);
    }

    return result;
}

QString WMFServicePlugin::deviceDescription(const QByteArray &service, const QByteArray &device)
{
    if (service == Q_MEDIASERVICE_CAMERA) {
        addRefCount();
        const QList<WMFVideoDeviceInfo> &devs = WMFVideoDeviceControl::availableDevices();
        releaseRefCount();
        for (const WMFVideoDeviceInfo &info : devs) {
            if (info.first == device)
                return info.second;
        }
    }
    return QString();
}

