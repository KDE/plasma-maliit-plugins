include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch MImServer
DEPENDPATH += .
INCLUDEPATH += . \
               ../stubs/

contains(CONFIG, noreactionmap) {
} else {
    DEFINES += HAVE_REACTIONMAP
    CONFIG += meegoreactionmap
}

include(../common_check.pri)

LIBS += -Wl,-rpath=$${MALIIT_PLUGINS_DIR} -lmeego-keyboard

# Input
HEADERS += ut_mvirtualkeyboard.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h \
           ../stubs/mreactionmaptester.h

SOURCES += ut_mvirtualkeyboard.cpp \
           ../stubs/fakegconf.cpp

