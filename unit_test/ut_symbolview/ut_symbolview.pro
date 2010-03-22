TEMPLATE = app
CONFIG += QtTest Dui DuiImServer duiimframework duireactionmap
DEPENDPATH += .
INCLUDEPATH += 	. \
		../stubs/ \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/dui-im-plugins/ -lduikeyboard

# Input
HEADERS += ut_symbolview.h \
           ../stubs/duigconfitem_stub.h \
           ../stubs/fakegconf.h \
           ../stubs/duireactionmaptester.h

SOURCES += ut_symbolview.cpp \
           ../stubs/fakegconf.cpp

include(../common_check.pri)
