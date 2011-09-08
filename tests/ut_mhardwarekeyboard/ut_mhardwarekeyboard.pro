include(../common_top.pri)

TEMPLATE = app
CONFIG += meegotouch MImServer
DEPENDPATH += .
INCLUDEPATH += . \
        ../../m-keyboard/ \
        ../stubs/


LIBS += -Wl,-rpath=$${MALIIT_PLUGINS_DIR} -L../../m-keyboard/ -lmeego-keyboard -lX11

# Input
HEADERS += ut_mhardwarekeyboard.h \
           mxkb_stub.h \
           testinputmethodhost.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h

SOURCES += ut_mhardwarekeyboard.cpp \
           ../stubs/fakegconf.cpp

include(../common_check.pri)
