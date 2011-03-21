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

LIBS += -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard


HEADERS += ut_mtoolbarbutton.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h \


SOURCES += ut_mtoolbarbutton.cpp \
           ../stubs/fakegconf.cpp \

target.files += \
           $$TARGET \
           testtoolbar.xml \

include(../common_check.pri)
