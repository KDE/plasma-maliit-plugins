RENDERER_DIR = ./renderer

HEADERS += \
    renderer.h \
    keyrenderer.h \
    keyareaitem.h \
    keyitem.h \
    abstractbackgroundbuffer.h \
    graphicsview.h \

SOURCES += \
    renderer.cpp \
    keyrenderer.cpp \
    keyareaitem.cpp \
    keyitem.cpp \
    abstractbackgroundbuffer.cpp \
    graphicsview.cpp \

INCLUDEPATH += $$RENDERER_DIR
DEPENDPATH += $$RENDERER_DIR


