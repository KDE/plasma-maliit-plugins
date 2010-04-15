TEMPLATE = app
CONFIG += QtTest meegotouch MImServer meegoimframework
DEPENDPATH += .
INCLUDEPATH += 	. \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard


# Input
HEADERS += ut_notification.h
SOURCES += ut_notification.cpp

include(../common_check.pri)

CSS_DATA = test.css
css_data.path = /usr/share/m-keyboard-tests/ut_notification/
css_data.files = $$CSS_DATA

INSTALLS += css_data

