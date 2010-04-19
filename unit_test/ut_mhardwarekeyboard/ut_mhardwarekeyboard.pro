TEMPLATE = app
CONFIG += QtTest meegotouch MImServer meegoimframework
DEPENDPATH += .
INCLUDEPATH += . \
        ../../m-keyboard/ \
        ../stubs/


LIBS += -Wl,-rpath=/usr/lib/meego-im-plugins/ -L../../m-keyboard/ -lmeego-keyboard

# Input
HEADERS += ut_mhardwarekeyboard.h \
           testinputcontextconnection.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h

SOURCES += ut_mhardwarekeyboard.cpp \
           ../stubs/fakegconf.cpp

include(../common_check.pri)
