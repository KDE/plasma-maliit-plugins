include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch MImServer meegoimframework
DEPENDPATH += .
INCLUDEPATH += 	. \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard

# Input
HEADERS += ut_layoutdata.h
SOURCES += ut_layoutdata.cpp

include(../common_check.pri)
