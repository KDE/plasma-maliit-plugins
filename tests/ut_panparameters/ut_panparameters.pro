include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch
DEPENDPATH += .
INCLUDEPATH += .

include(../common_check.pri)

LIBS += -Wl,-rpath=$${MALIIT_PLUGINS_DIR} -lmeego-keyboard

# Input
HEADERS += ut_panparameters.h \
           $$COMMON_DIR/panparameters.h \

SOURCES += ut_panparameters.cpp \
#           $$COMMON_DIR/panparameters.cpp \

