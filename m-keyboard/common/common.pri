COMMON_DIR = ./common

INSTALL_HEADERS += \
    $$COMMON_DIR/vkbdatakey.h \
    $$COMMON_DIR/keyevent.h \

HEADERS += \
    $$COMMON_DIR/mkeyboardcommon.h \
    $$COMMON_DIR/keyboarddata.h \
    $$COMMON_DIR/keyevent.h \
    $$COMMON_DIR/layoutdata.h \
    $$COMMON_DIR/layoutsmanager.h \
    $$COMMON_DIR/limitedtimer.h \
    $$COMMON_DIR/toolbardata.h \
    $$COMMON_DIR/toolbarwidget.h \
    $$COMMON_DIR/toolbarmanager.h \
    $$COMMON_DIR/vkbdatakey.h \
    $$COMMON_DIR/mxkb.h \
    $$COMMON_DIR/mhardwarekeyboard.h \
    $$COMMON_DIR/hwkbcharloops.h \
    $$COMMON_DIR/hwkbcharloopsmanager.h \
    $$COMMON_DIR/keyeventhandler.h \

SOURCES += \
    $$COMMON_DIR/keyboarddata.cpp\
    $$COMMON_DIR/keyevent.cpp\
    $$COMMON_DIR/layoutdata.cpp\
    $$COMMON_DIR/layoutsmanager.cpp\
    $$COMMON_DIR/limitedtimer.cpp\
    $$COMMON_DIR/toolbardata.cpp\
    $$COMMON_DIR/toolbarwidget.cpp \
    $$COMMON_DIR/toolbarmanager.cpp\
    $$COMMON_DIR/vkbdatakey.cpp\
    $$COMMON_DIR/mxkb.cpp \
    $$COMMON_DIR/mhardwarekeyboard.cpp \
    $$COMMON_DIR/hwkbcharloops.cpp \
    $$COMMON_DIR/hwkbcharloopsmanager.cpp \
    $$COMMON_DIR/keyeventhandler.cpp \

INCLUDEPATH += $$COMMON_DIR
DEPENDPATH += $$COMMON_DIR

