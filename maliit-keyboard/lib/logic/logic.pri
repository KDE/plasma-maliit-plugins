LOGIC_DIR = ./logic

HEADERS += \
    layoutupdater.h \
    keyboardloader.h \
    style.h \

SOURCES += \
    layoutupdater.cpp \
    keyboardloader.cpp \
    style.cpp \

DEPENDPATH += $$LOGIC_DIR

include(state-machines/state-machines.pri)
