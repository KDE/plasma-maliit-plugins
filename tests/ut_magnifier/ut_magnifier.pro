include(../common_top.pri)

TEMPLATE = app
CONFIG += meegotouch

include(../common_check.pri)

LIBS += -L/usr/lib/ -Wl,-rpath=$${MALIIT_PLUGINS_DIR} -lmeego-keyboard

HEADERS += \
    ut_magnifier.h \

SOURCES += \
    ut_magnifier.cpp \

RESOURCES += \
    ut_magnifier.qrc \
