LOGIC_DIR = ./logic

HEADERS += \
    layoutupdater.h \
    keyboardloader.h \

SOURCES += \
    layoutupdater.cpp

# This is temporary only, just to have anything working as xml LLL is still WIP.
use-xml-lll {
    SOURCES += xmlkeyboardloader.cpp
} else {
    SOURCES += keyboardloader.cpp
}

DEPENDPATH += $$LOGIC_DIR

include(state-machines/state-machines.pri)
