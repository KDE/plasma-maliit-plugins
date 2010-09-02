TEMPLATE = app
CONFIG += meegotouch
DEPENDPATH += .
INCLUDEPATH += . \

include(../common_check.pri)

# Input
HEADERS += ut_vkbdatakey.h \
           $$COMMON_DIR/vkbdatakey.h \
           $$WIDGETS_DIR/mvirtualkeyboardstyle.h \

LIBS += $$MKEYBOARD_DIR/.obj/gen_mvirtualkeyboardstyledata.o \

SOURCES += ut_vkbdatakey.cpp \
           $$COMMON_DIR/vkbdatakey.cpp \
           $$COMMON_DIR/keyevent.cpp \

