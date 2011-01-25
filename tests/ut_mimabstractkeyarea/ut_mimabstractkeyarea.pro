include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch MImServer meegoimframework
DEPENDPATH += .
INCLUDEPATH += 	. \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard
#LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard


# Input
HEADERS += ut_mimabstractkeyarea.h \
           ../../m-keyboard/widgets/popupfactory.h \
           ../../m-keyboard/widgets/mimabstractkeyarea.h \

SOURCES += ut_mimabstractkeyarea.cpp \
           ../../m-keyboard/widgets/popupfactory.cpp \
           ../../m-keyboard/widgets/mimabstractkeyarea.cpp \

VKB_TEST_DATA = layouts/*.xml
vkb_test_data.path = /usr/share/meegotouch/virtual-keyboard/layouts
vkb_test_data.files = $$VKB_TEST_DATA
INSTALLS += vkb_test_data

include(../common_check.pri)
