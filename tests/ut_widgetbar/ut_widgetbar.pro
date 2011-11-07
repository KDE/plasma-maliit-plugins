include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch

LIBS += -lmeegotouchviews

LIBS += -lmeegotouchviews -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard

include(../common_check.pri)

SOURCES += ut_widgetbar.cpp \

HEADERS += ut_widgetbar.h \

