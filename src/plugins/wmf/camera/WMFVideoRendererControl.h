#ifndef ESCAPIVIDEORENDERERCONTROL_H
#define ESCAPIVIDEORENDERERCONTROL_H

#include <QVideoRendererControl>

class WMFCameraSession;

class WMFVideoRendererControl : public QVideoRendererControl
{
    Q_OBJECT
public:
    WMFVideoRendererControl(WMFCameraSession* session, QObject* parent = nullptr);

    // QVideoRendererControl
    virtual void setSurface(QAbstractVideoSurface *surface) override;
    virtual QAbstractVideoSurface *	surface() const override;

private:
    WMFCameraSession* m_session;
};

#endif // ESCAPIVIDEORENDERERCONTROL_H
