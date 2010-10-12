TEMPLATE = app
CONFIG += QtTest meegotouch
DEPENDPATH += .
INCLUDEPATH += .

include(../common_check.pri)

STYLE_HEADERS += $$WIDGETS_DIR/keybuttonareastyle.h

# Input
HEADERS += ut_keybutton.h \
           $$WIDGETS_DIR/ikeybutton.h \
           $$WIDGETS_DIR/singlewidgetbutton.h \
           $$COMMON_DIR/vkbdatakey.h \
           $$COMMON_DIR/keyevent.h \
           $$STYLE_HEADERS

SOURCES += ut_keybutton.cpp \
           $$WIDGETS_DIR/singlewidgetbutton.cpp \
           $$COMMON_DIR/vkbdatakey.cpp \
           $$COMMON_DIR/keyevent.cpp

