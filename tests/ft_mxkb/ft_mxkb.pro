include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch MImServer
DEPENDPATH += .
INCLUDEPATH += . \
        ../../m-keyboard/


LIBS += -Wl,-rpath=/usr/lib/meego-im-plugins/ -L../../m-keyboard/ -lmeego-keyboard -lX11

# Input
HEADERS += ft_mxkb.h

SOURCES += ft_mxkb.cpp

include(../common_check.pri)
