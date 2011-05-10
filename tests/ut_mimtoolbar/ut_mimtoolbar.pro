include(../common_top.pri)

TEMPLATE = app

contains(CONFIG, nomeegotouch) {
} else {
    DEFINES += HAVE_MEEGOTOUCH
    CONFIG += meegotouch
}

contains(CONFIG, noreactionmap) {
} else {
    DEFINES += HAVE_REACTIONMAP
    CONFIG += meegoreactionmap
}

DEPENDPATH += .
INCLUDEPATH += . \
               ../stubs/

LIBS += -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard


HEADERS += ut_mimtoolbar.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h \
           ../stubs/mreactionmaptester.h \


SOURCES += ut_mimtoolbar.cpp \
           ../stubs/fakegconf.cpp \

target.files += \
           $$TARGET \
           testtoolbar.xml \
           testtoolbar2.xml \
           testtoolbar4.xml \

include(../common_check.pri)
