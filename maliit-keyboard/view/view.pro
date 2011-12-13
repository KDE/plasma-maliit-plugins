include(../../config.pri)

VERSION = 0.2.0
TARGET = maliit-keyboard-view
TEMPLATE = lib
QT += core gui
INCLUDEPATH=../lib
LIB += -L../lib -lmaliit-keyboard

HEADERS += \
    glass.h \
    renderer.h \
    keyrenderer.h \
    keyareaitem.h \
    keyitem.h \
    abstractbackgroundbuffer.h \
    graphicsview.h \

SOURCES += \
    glass.cpp \
    renderer.cpp \
    keyrenderer.cpp \
    keyareaitem.cpp \
    keyitem.cpp \
    abstractbackgroundbuffer.cpp \
    graphicsview.cpp \

target.path += $$PREFIX/usr/lib
INSTALLS += target

