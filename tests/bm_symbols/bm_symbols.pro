include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch MImServer
DEPENDPATH += .
INCLUDEPATH += . \

LIBS += -L/usr/lib -Wl,-rpath=$${MALIIT_PLUGINS_DIR} -lmeego-keyboard


# Input
HEADERS += bm_symbols.h
SOURCES += bm_symbols.cpp

include(../common_check.pri)
