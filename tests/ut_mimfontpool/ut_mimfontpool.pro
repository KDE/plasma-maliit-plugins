include(../common_top.pri)

TEMPLATE = app

contains(CONFIG, nomeegotouch) {
} else {
    DEFINES += HAVE_MEEGOTOUCH
    CONFIG += meegotouch
}

DEPENDPATH += .
INCLUDEPATH += . \
               ../stubs/

LIBS += -Wl,-rpath=$${MALIIT_PLUGINS_DIR} -lmeego-keyboard


HEADERS += ut_mimfontpool.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h \


SOURCES += ut_mimfontpool.cpp \
           ../stubs/fakegconf.cpp \

include(../common_check.pri)
