TEMPLATE = app
CONFIG += QtTest meegotouch
DEPENDPATH += .
INCLUDEPATH += .

include(../common_check.pri)

STYLE_HEADERS += $$WIDGETS_DIR/mimabstractkeyareastyle.h

# Input
HEADERS += ut_mimkey.h \
           $$WIDGETS_DIR/mimabstractkey.h \
           $$WIDGETS_DIR/mimkey.h \
           $$COMMON_DIR/mimkeymodel.h \
           $$COMMON_DIR/keyevent.h \
           $$STYLE_HEADERS

SOURCES += ut_mimkey.cpp \
           $$WIDGETS_DIR/mimabstractkey.cpp \
           $$WIDGETS_DIR/mimkey.cpp \
           $$COMMON_DIR/mimkeymodel.cpp \
           $$COMMON_DIR/keyevent.cpp

