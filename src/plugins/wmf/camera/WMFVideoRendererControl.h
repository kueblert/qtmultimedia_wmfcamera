#ifndef WMFVIDEORENDERERCONTROL_H
#define WMFVIDEORENDERERCONTROL_H

#include <qvideorenderercontrol.h>
#include "wmfcamerasession.h"

QT_BEGIN_NAMESPACE

class WMFVideoRendererControl : public QVideoRendererControl
{
public:
    WMFVideoRendererControl(WMFCameraSession* session, QObject* parent = 0);
    ~WMFVideoRendererControl() = default;

    QAbstractVideoSurface* surface() const;
    void setSurface(QAbstractVideoSurface* surface);
private:
    QAbstractVideoSurface* m_surface;
    WMFCameraSession* m_session;
};

QT_END_NAMESPACE

#endif // WMFVIDEORENDERERCONTROL_H
