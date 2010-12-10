include(../common_top.pri)

TEMPLATE = app

CONFIG += meegotouch

include(../common_check.pri)

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard

SOURCES += ut_horizontalswitcher.cpp \

HEADERS += ut_horizontalswitcher.h \

