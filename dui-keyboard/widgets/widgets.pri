WIDGETS_DIR = ./widgets

INSTALL_HEADERS += \
    $$WIDGETS_DIR/ikeybutton.h \
    $$WIDGETS_DIR/popupbase.h \
    $$WIDGETS_DIR/duivirtualkeyboardstyle.h \
    $$WIDGETS_DIR/popupplugin.h \

STYLE_HEADERS += \
    $$WIDGETS_DIR/buttonbarstyle.h \
    $$WIDGETS_DIR/flickupbuttonstyle.h \
    $$WIDGETS_DIR/duivirtualkeyboardstyle.h \

HEADERS += \
    $$STYLE_HEADERS \
    $$WIDGETS_DIR/buttonbar.h \
    $$WIDGETS_DIR/duiimcorrectioncandidatewindow.h \
    $$WIDGETS_DIR/duiimcorrectioncandidatewidget.h \
    $$WIDGETS_DIR/duiimtoolbar.h \
    $$WIDGETS_DIR/duivirtualkeyboard.h \
    $$WIDGETS_DIR/flickupbutton.h \
    $$WIDGETS_DIR/flickupbuttonview.h \
    $$WIDGETS_DIR/horizontalswitcher.h \
    $$WIDGETS_DIR/layoutmenu.h \
    $$WIDGETS_DIR/notification.h \
    $$WIDGETS_DIR/symbolview.h \
    $$WIDGETS_DIR/keybuttonarea.h \
    $$WIDGETS_DIR/ikeybutton.h \
    $$WIDGETS_DIR/singlewidgetbutton.h \
    $$WIDGETS_DIR/singlewidgetbuttonarea.h \
    $$WIDGETS_DIR/duibuttonarea.h \
    $$WIDGETS_DIR/popupbase.h \
    $$WIDGETS_DIR/popupfactory.h \
    $$WIDGETS_DIR/popupplugin.h \


SOURCES += \
    $$WIDGETS_DIR/buttonbar.cpp \
    $$WIDGETS_DIR/duiimcorrectioncandidatewindow.cpp \
    $$WIDGETS_DIR/duiimcorrectioncandidatewidget.cpp \
    $$WIDGETS_DIR/duiimtoolbar.cpp \
    $$WIDGETS_DIR/duivirtualkeyboard.cpp \
    $$WIDGETS_DIR/flickupbutton.cpp \
    $$WIDGETS_DIR/flickupbuttonview.cpp \
    $$WIDGETS_DIR/horizontalswitcher.cpp \
    $$WIDGETS_DIR/layoutmenu.cpp \
    $$WIDGETS_DIR/notification.cpp \
    $$WIDGETS_DIR/symbolview.cpp \
    $$WIDGETS_DIR/keybuttonarea.cpp \
    $$WIDGETS_DIR/singlewidgetbutton.cpp \
    $$WIDGETS_DIR/singlewidgetbuttonarea.cpp \
    $$WIDGETS_DIR/duibuttonarea.cpp \
    $$WIDGETS_DIR/popupbase.cpp \
    $$WIDGETS_DIR/popupfactory.cpp \

INCLUDEPATH += $$WIDGETS_DIR
DEPENDPATH += $$WIDGETS_DIR

