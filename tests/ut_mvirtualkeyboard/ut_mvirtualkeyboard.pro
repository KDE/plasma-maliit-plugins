include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch MImServer meegoimframework meegoreactionmap
DEPENDPATH += .
INCLUDEPATH += . \
		../stubs/


include(../common_check.pri)

LIBS += -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard

# Input
HEADERS += ut_mvirtualkeyboard.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h \
           ../stubs/mreactionmaptester.h

SOURCES += ut_mvirtualkeyboard.cpp \
           ../stubs/fakegconf.cpp

