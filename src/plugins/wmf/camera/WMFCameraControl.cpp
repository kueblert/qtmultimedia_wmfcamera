#include "WMFCameraControl.h"
#include "WMFCameraSession.h"

WMFCameraControl::WMFCameraControl(WMFCameraSession* session)
    : QCameraControl(nullptr)
    , m_state(QCamera::UnloadedState)
    , m_session(session)
{

}

void WMFCameraControl::setState(QCamera::State state)
{
    //qDebug() << "State change request from " << m_state << " to " << state;
    if (m_state == state)
        return;

    bool succeeded = false;
    switch (state) {
    case QCamera::UnloadedState:
        succeeded = m_session->unload();
        break;

    case QCamera::LoadedState:
        if (m_state == QCamera::UnloadedState){
            succeeded = m_session->load();
            //qDebug() << "Loading was " << succeeded;
        }
        if (m_state == QCamera::ActiveState){
            succeeded = m_session->stopPreview();
        }
        break;

    case QCamera::ActiveState:
        if (m_state == QCamera::UnloadedState) {
            if (!m_session->load())
                break;
            m_state = QCamera::LoadedState;
        }

        if (state == QCamera::ActiveState){
            succeeded = m_session->startPreview();
            //qDebug() << "startPreview " << succeeded;
        }
        else
            succeeded = m_session->stopPreview();

        break;
    }

    if (succeeded) {
        m_state = state;
        emit stateChanged(m_state);
    }
}

bool WMFCameraControl::isCaptureModeSupported(QCamera::CaptureModes mode) const
{
    bool bCaptureSupported = false;
    switch (mode) {
    case QCamera::CaptureStillImage:
        bCaptureSupported = false;
        break;
    case QCamera::CaptureVideo:
        bCaptureSupported = false;
        break;
    }
    return bCaptureSupported;
}

void WMFCameraControl::setCaptureMode(QCamera::CaptureModes mode)
{
    if (m_captureMode != mode && isCaptureModeSupported(mode)) {
        m_captureMode = mode;
        emit captureModeChanged(mode);
    }
}

QCamera::Status WMFCameraControl::status() const
{
    return m_session->status();
}
