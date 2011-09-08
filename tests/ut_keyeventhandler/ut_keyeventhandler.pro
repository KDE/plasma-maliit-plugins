include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch MImServer
DEPENDPATH += .
INCLUDEPATH += 	. \

LIBS += -L/usr/lib -Wl,-rpath=$${MALIIT_PLUGINS_DIR} -lmeego-keyboard
#LIBS += -L/usr/lib -Wl,-rpath=$${MALIIT_PLUGINS_DIR} -lmeego-keyboard


# Input
HEADERS += ut_keyeventhandler.h
SOURCES += ut_keyeventhandler.cpp

include(../common_check.pri)
