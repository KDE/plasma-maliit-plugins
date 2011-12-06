LOGIC_DIR = ./logic

HEADERS += \
    layoutupdater.h \
    keyboardloader.h \

SOURCES += \
    layoutupdater.cpp \
    keyboardloader.cpp \

DEPENDPATH += $$LOGIC_DIR

include(state-machines/state-machines.pri)
