TEMPLATE = app
CONFIG += QtTest Dui DuiImServer duiimframework
DEPENDPATH += .
INCLUDEPATH += 	. \

include(../common_check.pri)

# Input
HEADERS += ut_limitedtimer.h $$COMMON_DIR/limitedtimer.h
SOURCES += ut_limitedtimer.cpp $$COMMON_DIR/limitedtimer.cpp

