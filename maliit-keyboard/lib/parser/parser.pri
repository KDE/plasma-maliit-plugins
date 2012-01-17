PARSER_DIR = ./parser

HEADERS += \
    layoutparser.h \
    tagbinding.h \
    tagkeyboard.h \
    tagkey.h \
    taglayout.h \
    tagrowelement.h \
    tagrow.h \
    tagsection.h \
    tagspacer.h

SOURCES += \
    layoutparser.cpp \
    tagbinding.cpp \
    tagkeyboard.cpp \
    tagkey.cpp \
    taglayout.cpp \
    tagrow.cpp \
    tagrowelement.cpp \
    tagsection.cpp \
    tagspacer.cpp

DEPENDPATH += $$PARSER_DIR
