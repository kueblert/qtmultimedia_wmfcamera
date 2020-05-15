#include "WMFVideoRendererControl.h"

QT_BEGIN_NAMESPACE

WMFVideoRendererControl::WMFVideoRendererControl(WMFCameraSession* session, QObject* parent)
    : QVideoRendererControl(parent)
    , m_surface(nullptr)
    , m_session(session)
{

}

QAbstractVideoSurface* WMFVideoRendererControl::surface() const
{
    return m_surface;
}

void WMFVideoRendererControl::setSurface(QAbstractVideoSurface* surface)
{
    m_surface = surface;
    if (m_session)
        m_session->setSurface(m_surface);
}

QT_END_NAMESPACE
