TEMPLATE = app
CONFIG += QtTest Dui DuiImServer duiimframework
DEPENDPATH += .
INCLUDEPATH += 	. \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/dui-im-plugins/ -lduikeyboard


# Input
HEADERS += ut_notification.h
SOURCES += ut_notification.cpp

include(../common_check.pri)

CSS_DATA = test.css
css_data.path = /usr/share/dui-im-virtualkeyboard-tests/ut_notification/
css_data.files = $$CSS_DATA

INSTALLS += css_data

