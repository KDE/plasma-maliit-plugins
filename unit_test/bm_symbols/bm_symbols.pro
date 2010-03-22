TEMPLATE = app
CONFIG += QtTest Dui DuiImServer duiimframework
DEPENDPATH += .
INCLUDEPATH += . \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/dui-im-plugins/ -lduikeyboard


# Input
HEADERS += bm_symbols.h
SOURCES += bm_symbols.cpp

include(../common_check.pri)
