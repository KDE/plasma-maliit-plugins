TEMPLATE = app
CONFIG += QtTest meegotouch MImServer mimframework
DEPENDPATH += .
INCLUDEPATH += . \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/m-im-plugins/ -lmkeyboard


# Input
HEADERS += bm_symbols.h
SOURCES += bm_symbols.cpp

include(../common_check.pri)
