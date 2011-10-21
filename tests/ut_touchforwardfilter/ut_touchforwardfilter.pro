include(../common_top.pri)

TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += ut_touchforwardfilter.h \
           touchtargetwidget.h \
           $$WIDGETS_DIR/touchforwardfilter.h
SOURCES += ut_touchforwardfilter.cpp \
           $$WIDGETS_DIR/touchforwardfilter.cpp

include(../common_check.pri)
