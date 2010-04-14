TEMPLATE = app
CONFIG += QtTest meegotouch MImServer
DEPENDPATH += .
INCLUDEPATH += 	. \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/m-im-plugins/ -lmkeyboard
#LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/m-im-plugins/ -lmkeyboard


# Input
HEADERS += ut_keybuttonarea.h
SOURCES += ut_keybuttonarea.cpp

VKB_TEST_DATA = layouts/*.xml
vkb_test_data.path = /usr/share/meegotouch/virtual-keyboard/layouts
vkb_test_data.files = $$VKB_TEST_DATA
INSTALLS += vkb_test_data

include(../common_check.pri)
