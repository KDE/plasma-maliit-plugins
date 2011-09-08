include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch
LIBS += -Wl,-rpath=$${MALIIT_PLUGINS_DIR} -lmeego-keyboard

include(../common_check.pri)

SOURCES += ut_wordribbonhost.cpp \
           ../stubs/minputmethodhoststub.cpp \

HEADERS += ut_wordribbonhost.h \
           ../stubs/minputmethodhoststub.h \

