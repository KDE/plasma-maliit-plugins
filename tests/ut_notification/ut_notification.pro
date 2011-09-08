include(../common_top.pri)

CONFIG += QtTest meegotouch MImServer
DEPENDPATH += .
INCLUDEPATH += 	. \

LIBS += -L/usr/lib -Wl,-rpath=$${MALIIT_PLUGINS_DIR} -lmeego-keyboard


# Input
HEADERS += ut_notification.h
SOURCES += ut_notification.cpp

include(../common_check.pri)
