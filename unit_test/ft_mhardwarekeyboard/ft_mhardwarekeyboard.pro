TEMPLATE = app
CONFIG += QtTest Dui DuiImServer duiimframework
DEPENDPATH += .
INCLUDEPATH += . \
        ../../dui-keyboard/


LIBS += -Wl,-rpath=/usr/lib/dui-im-plugins/ -L../../dui-keyboard/ -lduikeyboard

# Input
HEADERS += ft_duihardwarekeyboard.h

SOURCES += ft_duihardwarekeyboard.cpp

include(../common_check.pri)
