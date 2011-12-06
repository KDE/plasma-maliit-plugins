STATE_MACHINES_DIR = $$LOGIC_DIR/state-machines

HEADERS += \
    abstractstatemachine.h \
    shiftmachine.h \
    viewmachine.h \

SOURCES += \
    abstractstatemachine.cpp \
    shiftmachine.cpp \
    viewmachine.cpp \

DEPENDPATH += $$STATE_MACHINES_DIR
