include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch meegoimframework

include(../common_check.pri)

STYLE_HEADERS += $$WIDGETS_DIR/widgetbarstyle.h \
                 $$WIDGETS_DIR/mtoolbarbuttonstyle.h \
                 $$WIDGETS_DIR/mtoolbarlabelstyle.h \

SOURCES += ut_widgetbar.cpp \
           $$WIDGETS_DIR/widgetbar.cpp \
           $$WIDGETS_DIR/mtoolbarbutton.cpp \
           $$WIDGETS_DIR/mtoolbarlabel.cpp \
           $$WIDGETS_DIR/mtoolbarbuttonview.cpp \
           $$WIDGETS_DIR/mtoolbarlabelview.cpp \

HEADERS += ut_widgetbar.h \
           $$WIDGETS_DIR/widgetbar.h \
           $$WIDGETS_DIR/mtoolbarbutton.h \
           $$WIDGETS_DIR/mtoolbarlabel.h \
           $$WIDGETS_DIR/mtoolbarbuttonview.h \
           $$WIDGETS_DIR/mtoolbarlabelview.h \
           $$STYLE_HEADERS \

