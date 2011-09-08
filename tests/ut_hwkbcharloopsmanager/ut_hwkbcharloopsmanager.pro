include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch MImServer
DEPENDPATH += .
INCLUDEPATH += 	. \
		../stubs/

LIBS += -Wl,-rpath=$${MALIIT_PLUGINS_DIR} -lmeego-keyboard

# Input
HEADERS += ut_hwkbcharloopsmanager.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h

SOURCES += ut_hwkbcharloopsmanager.cpp \
           ../stubs/fakegconf.cpp

include(../common_check.pri)
