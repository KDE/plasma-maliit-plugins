TEMPLATE = app
CONFIG += QtTest meegotouch

include(../common_check.pri)

STYLE_HEADERS += $$WIDGETS_DIR/buttonbarstyle.h

SOURCES += ut_buttonbar.cpp \
           $$WIDGETS_DIR/buttonbar.cpp

HEADERS += ut_buttonbar.h \
           $$WIDGETS_DIR/buttonbar.h \
           $$STYLE_HEADERS \

