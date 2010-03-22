TEMPLATE = app
CONFIG += QtTest Dui DuiImServer duiimframework
DEPENDPATH += .
INCLUDEPATH += 	. \
		../stubs/

LIBS += -Wl,-rpath=/usr/lib/dui-im-plugins/ -lduikeyboard

# Input
HEADERS += ut_layoutsmanager.h \
           ../stubs/duigconfitem_stub.h \
           ../stubs/fakegconf.h

SOURCES += ut_layoutsmanager.cpp \
           ../stubs/fakegconf.cpp

include(../common_check.pri)
