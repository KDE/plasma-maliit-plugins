include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch meegoimframework
LIBS += -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard

include(../common_check.pri)

SOURCES += ut_wordribbonhost.cpp \
           ../stubs/minputmethodhoststub.cpp \

HEADERS += ut_wordribbonhost.h \
           ../stubs/minputmethodhoststub.h \

