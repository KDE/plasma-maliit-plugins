include(../common_top.pri)

TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .

CONFIG += QtTest meegotouch
LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard

HEADERS += \
    ut_magnifierhost.h \

SOURCES += \
    ut_magnifierhost.cpp \

include(../common_check.pri)
