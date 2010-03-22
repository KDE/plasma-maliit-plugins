TEMPLATE = app
CONFIG += QtTest Dui DuiImServer duiimframework
DEPENDPATH += .
INCLUDEPATH += 	. \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/dui-im-plugins/ -lduikeyboard

# Input
HEADERS += bm_keybuttonarea.h
SOURCES += bm_keybuttonarea.cpp

include(../common_check.pri)
