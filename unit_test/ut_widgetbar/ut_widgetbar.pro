TEMPLATE = app
CONFIG += QtTest meegotouch

include(../common_check.pri)

STYLE_HEADERS += $$WIDGETS_DIR/widgetbarstyle.h

SOURCES += ut_widgetbar.cpp \
           $$WIDGETS_DIR/widgetbar.cpp

HEADERS += ut_widgetbar.h \
           $$WIDGETS_DIR/widgetbar.h \
           $$STYLE_HEADERS \

