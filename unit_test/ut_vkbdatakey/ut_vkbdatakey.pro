TEMPLATE = app
CONFIG += QtTest Dui
DEPENDPATH += .
INCLUDEPATH += . \

include(../common_check.pri)

# Input
HEADERS += ut_vkbdatakey.h \
           $$COMMON_DIR\vkbdatakey.h \

SOURCES += ut_vkbdatakey.cpp \
           $$COMMON_DIR\vkbdatakey.cpp \
           $$COMMON_DIR\keyevent.cpp \

