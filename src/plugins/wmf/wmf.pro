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

CONFIG(release) {
DESTDIR = C:/Users/kueblert/Documents/Look/build/GUI/release/mediaservice
}
CONFIG(debug) {
DESTDIR = C:/Users/kueblert/Documents/Look/build/GUI/debug/mediaservice
}
