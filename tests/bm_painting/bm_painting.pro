include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch MImServer
DEPENDPATH += .
INCLUDEPATH += .

LIBS += -L/usr/lib -Wl,-rpath=$${MALIIT_PLUGINS_DIR} -lmeego-keyboard

# Input
HEADERS += bm_painting.h \
           paintrunner.h \
           loggingwindow.h \

SOURCES += bm_painting.cpp \
           paintrunner.cpp \
           loggingwindow.cpp \

include(../common_check.pri)
