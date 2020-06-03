#include "WMFVideoRendererControl.h"
#include "WMFCameraSession.h"

WMFVideoRendererControl::WMFVideoRendererControl(WMFCameraSession* session, QObject* parent)
    : QVideoRendererControl(parent)
    , m_session(session)
{

}
void WMFVideoRendererControl::setSurface(QAbstractVideoSurface *surface) {
    assert(m_session);
    m_session->setSurface(surface);
}

QAbstractVideoSurface *	WMFVideoRendererControl::surface() const {
    assert(m_session);
    return m_session->surface();
}
