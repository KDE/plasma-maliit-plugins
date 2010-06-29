TEMPLATE = app
CONFIG += QtTest meegotouch MImServer meegoimframework
DEPENDPATH += .
INCLUDEPATH += 	. \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard
#LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard


# Input
HEADERS += ut_keybuttonarea.h ../ut_flickrecognizer/flickutil.h
SOURCES += ut_keybuttonarea.cpp ../ut_flickrecognizer/flickutil.cpp

VKB_TEST_DATA = layouts/*.xml
vkb_test_data.path = /usr/share/meegotouch/virtual-keyboard/layouts
vkb_test_data.files = $$VKB_TEST_DATA
INSTALLS += vkb_test_data

include(../common_check.pri)
