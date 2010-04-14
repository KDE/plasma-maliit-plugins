TEMPLATE = app
CONFIG += QtTest meegotouch MImServer mimframework duireactionmap
DEPENDPATH += .
INCLUDEPATH += 	. \
		../stubs/ \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/m-im-plugins/ -lmkeyboard

# Input
HEADERS += ut_symbolview.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h \
           ../stubs/duireactionmaptester.h

SOURCES += ut_symbolview.cpp \
           ../stubs/fakegconf.cpp

include(../common_check.pri)
