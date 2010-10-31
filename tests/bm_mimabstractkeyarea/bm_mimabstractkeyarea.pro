TEMPLATE = app
CONFIG += QtTest meegotouch MImServer meegoimframework
DEPENDPATH += .
INCLUDEPATH += 	. \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard

# Input
HEADERS += bm_mimabstractkeyarea.h
SOURCES += bm_mimabstractkeyarea.cpp

include(../common_check.pri)
