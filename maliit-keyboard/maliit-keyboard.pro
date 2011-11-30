include(../config.pri)

VERSION = 0.2.0
TARGET = maliit-keyboard
TEMPLATE = lib

contains(QT_MAJOR_VERSION, 4) {
    QT = core gui declarative
} else {
    QT = core gui widgets declarative
}

include(renderer/renderer.pri)
include(models/models.pri)
include(glass/glass.pri)

target.path += $$PREFIX/usr/lib
INSTALLS += target
