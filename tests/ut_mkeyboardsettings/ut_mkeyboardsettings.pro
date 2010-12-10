include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch MImServer
DEPENDPATH += .
INCLUDEPATH += 	. \
		../stubs/

LIBS += -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard

# Input
HEADERS += ut_mkeyboardsettings.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h

SOURCES += ut_mkeyboardsettings.cpp \
           ../stubs/fakegconf.cpp

include(../common_check.pri)
