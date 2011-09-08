include(../common_top.pri)

TEMPLATE = app
CONFIG += meegotouch

DEPENDPATH += .
INCLUDEPATH += . \
               ../stubs/

LIBS += -Wl,-rpath=$${MALIIT_PLUGINS_DIR} -lmeego-keyboard


HEADERS += ut_sharedhandlearea.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h \


SOURCES += ut_sharedhandlearea.cpp \
           ../stubs/fakegconf.cpp \

target.files += \
           $$TARGET \

include(../common_check.pri)
