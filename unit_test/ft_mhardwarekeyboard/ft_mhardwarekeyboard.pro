TEMPLATE = app
CONFIG += QtTest meegotouch MImServer mimframework
DEPENDPATH += .
INCLUDEPATH += . \
        ../../m-keyboard/


LIBS += -Wl,-rpath=/usr/lib/m-im-plugins/ -L../../m-keyboard/ -lmkeyboard

# Input
HEADERS += ft_mhardwarekeyboard.h

SOURCES += ft_mhardwarekeyboard.cpp

include(../common_check.pri)
