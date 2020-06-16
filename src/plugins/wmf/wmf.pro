TARGET = wmfengine
QT += multimedia-private network

CONFIG += c++11

win32:!qtHaveModule(opengl) {
    LIBS_PRIVATE += -lgdi32 -luser32
}

INCLUDEPATH += .

HEADERS += \
    wmfserviceplugin.h \
    mfstream.h \
    sourceresolver.h

SOURCES += \
    wmfserviceplugin.cpp \
    mfstream.cpp \
    sourceresolver.cpp

qtConfig(wmf-player): include (player/player.pri)
include (decoder/decoder.pri)
include (camera/camera.pri)

OTHER_FILES += \
    wmf.json \
    wmf_audiodecode.json

PLUGIN_TYPE = mediaservice
PLUGIN_CLASS_NAME = WMFServicePlugin
load(qt_plugin)

#DESTDIR = $$[QT_INSTALL_PLUGINS]/mediaservice


Release:BUILD_TYPE="release"
Debug:BUILD_TYPE="debug"
DESTDIR = ../../../../../../GUI/$$BUILD_TYPE/mediaservice
