include(../common_top.pri)

CONFIG += QtTest meegotouch MImServer meegoimframework
DEPENDPATH += .
INCLUDEPATH += 	. \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard


# Input
HEADERS += ut_notification.h
SOURCES += ut_notification.cpp

include(../common_check.pri)
