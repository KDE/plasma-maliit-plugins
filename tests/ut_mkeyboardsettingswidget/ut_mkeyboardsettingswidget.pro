include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch
DEPENDPATH += .
INCLUDEPATH += 	. \
		../stubs/

LIBS += -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard

# Input
SOURCES += ut_mkeyboardsettingswidget.cpp \
           ../stubs/fakegconf.cpp \

HEADERS += ut_mkeyboardsettingswidget.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h \


include(../common_check.pri)
