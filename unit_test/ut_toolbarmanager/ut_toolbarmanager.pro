TEMPLATE = app
CONFIG += QtTest Dui
DEPENDPATH += .
INCLUDEPATH += 	. \
		../../dui-keyboard/ \
		../stubs/

LIBS += -Wl,-rpath=/usr/lib/dui-im-plugins/ -L../../dui-keyboard -lduikeyboard

# Input
HEADERS += ut_toolbarmanager.h \

SOURCES += ut_toolbarmanager.cpp \

include(../common_check.pri)
