TEMPLATE = app
CONFIG += QtTest Dui DuiImServer
DEPENDPATH += .
INCLUDEPATH += 	. \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/dui-im-plugins/ -lduikeyboard
#LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/dui-im-plugins/ -lduikeyboard


# Input
HEADERS += ut_keybuttonarea.h
SOURCES += ut_keybuttonarea.cpp

VKB_TEST_DATA = layouts/*.xml
vkb_test_data.path = /usr/share/dui/virtual-keyboard/layouts
vkb_test_data.files = $$VKB_TEST_DATA
INSTALLS += vkb_test_data

include(../common_check.pri)
