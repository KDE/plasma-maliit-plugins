TEMPLATE = app
CONFIG += QtTest Dui DuiImServer duiimframework
DEPENDPATH += .
INCLUDEPATH += . \
        ../../dui-keyboard/ \
        ../stubs/


LIBS += -Wl,-rpath=/usr/lib/dui-im-plugins/ -L../../dui-keyboard/ -lduikeyboard

# Input
HEADERS += ut_duihardwarekeyboard.h \
           ../stubs/duigconfitem_stub.h \
           ../stubs/fakegconf.h

SOURCES += ut_duihardwarekeyboard.cpp \
           ../stubs/fakegconf.cpp

include(../common_check.pri)
