TEMPLATE = app
CONFIG += QtTest meegotouch MImServer mimframework
DEPENDPATH += .
INCLUDEPATH += 	. \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/m-im-plugins/ -lmkeyboard

# Input
HEADERS += bm_keybuttonarea.h
SOURCES += bm_keybuttonarea.cpp

include(../common_check.pri)
