TEMPLATE = app
CONFIG += QtTest meegotouch MImServer mimframework
DEPENDPATH += .
INCLUDEPATH += 	. \
		../stubs/

LIBS += -Wl,-rpath=/usr/lib/m-im-plugins/ -lmkeyboard

# Input
HEADERS += ut_hwkbcharloopsmanager.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h

SOURCES += ut_hwkbcharloopsmanager.cpp \
           ../stubs/fakegconf.cpp

include(../common_check.pri)
