include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch MImServer meegoimframework
DEPENDPATH += .
INCLUDEPATH += . \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard


# Input
HEADERS += bm_symbols.h
SOURCES += bm_symbols.cpp

include(../common_check.pri)
