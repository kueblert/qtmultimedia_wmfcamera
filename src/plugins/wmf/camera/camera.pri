INCLUDEPATH += $$PWD

win32: DEFINES += _CRT_SECURE_NO_WARNINGS

QMAKE_USE += wmf

HEADERS += \
    $$PWD/DeviceList.h \
    $$PWD/WMFCameraBackend.h \
    $$PWD/WMFCameraControl.h \
    $$PWD/WMFCameraExposureControl.h \
    $$PWD/WMFCameraImageProcessingControl.h \
    $$PWD/WMFCameraService.h \
    $$PWD/WMFCameraSession.h \
    $$PWD/WMFImageProcessingControls.h \
    $$PWD/WMFVideoDeviceSelectorControl.h \
    $$PWD/WMFVideoRendererControl.h \
    $$PWD/WMFViewfinderSettingsControl2.h \
    $$PWD/capture.h \
    $$PWD/choosedeviceparam.h \
    $$PWD/conversion.h \
    $$PWD/escapi_minimal.h \
    $$PWD/scopedrelease.h \
    $$PWD/videobufferlock.h

SOURCES += \
    $$PWD/DeviceList.cpp \
    $$PWD/WMFCameraBackend.cpp \
    $$PWD/WMFCameraControl.cpp \
    $$PWD/WMFCameraExposureControl.cpp \
    $$PWD/WMFCameraImageProcessingControl.cpp \
    $$PWD/WMFCameraService.cpp \
    $$PWD/WMFCameraSession.cpp \
    $$PWD/WMFImageProcessingControls.cpp \
    $$PWD/WMFVideoDeviceSelectorControl.cpp \
    $$PWD/WMFVideoRendererControl.cpp \
    $$PWD/WMFViewfinderSettingsControl2.cpp \
    $$PWD/capture.cpp \
    $$PWD/conversion.cpp \
    $$PWD/escapi_minimal.cpp \
    $$PWD/videobufferlock.cpp

LIBS += -lmfplat -lmf -lmfreadwrite -lmfuuid -lshlwapi
