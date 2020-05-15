INCLUDEPATH += $$PWD

win32: DEFINES += _CRT_SECURE_NO_WARNINGS

QMAKE_USE += wmf

HEADERS += \
    $$PWD/DeviceList.h \
    $$PWD/WMFCameraControl.h \
    $$PWD/WMFCameraViewfinderSettingsControl.h \
    $$PWD/WMFVideoDeviceControl.h \
    $$PWD/WMFVideoRendererControl.h \
    $$PWD/wmfcameraservice.h \
    $$PWD/wmfcamerasession.h

SOURCES += \
    $$PWD/DeviceList.cpp \
    $$PWD/WMFCameraControl.cpp \
    $$PWD/WMFCameraViewfinderSettingsControl.cpp \
    $$PWD/WMFVideoDeviceControl.cpp \
    $$PWD/WMFVideoRendererControl.cpp \
    $$PWD/wmfcameraservice.cpp \
    $$PWD/wmfcamerasession.cpp

LIBS += -luser32
