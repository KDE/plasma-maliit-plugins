TEMPLATE = app
CONFIG += QtTest meegotouch
DEPENDPATH += .
INCLUDEPATH += 	. \
		../../m-keyboard/ \
		../stubs/

LIBS += -Wl,-rpath=/usr/lib/meego-im-plugins/ -L../../m-keyboard -lmeego-keyboard

# Input
HEADERS += ut_toolbarmanager.h \

SOURCES += ut_toolbarmanager.cpp \

include(../common_check.pri)
