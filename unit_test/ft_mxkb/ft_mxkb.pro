TEMPLATE = app
CONFIG += QtTest meegotouch MImServer mimframework
DEPENDPATH += .
INCLUDEPATH += . \
        ../../m-keyboard/


LIBS += -Wl,-rpath=/usr/lib/m-im-plugins/ -L../../m-keyboard/ -lmkeyboard

# Input
HEADERS += ft_mxkb.h

SOURCES += ft_mxkb.cpp

include(../common_check.pri)
