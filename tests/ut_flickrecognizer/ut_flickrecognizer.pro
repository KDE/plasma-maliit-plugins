include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest
DEPENDPATH += .
INCLUDEPATH += 	.

LIBS += -L/usr/lib -Wl

# Input
HEADERS += ut_flickrecognizer.h \
           ../../m-keyboard/common/flickgesturerecognizer.h \
           ../../m-keyboard/common/flickgesture.h

SOURCES += ut_flickrecognizer.cpp \
           ../../m-keyboard/common/flickgesturerecognizer.cpp \
           ../../m-keyboard/common/flickgesture.cpp

include(../common_check.pri)
