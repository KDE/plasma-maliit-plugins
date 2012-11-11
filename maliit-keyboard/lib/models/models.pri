MODELS_DIR = ./models

HEADERS += \
    models/area.h \
    models/font.h \
    models/label.h \
    models/key.h \
    models/keyarea.h \
    models/keyareacontainer.h \
    models/keyboard.h \
    models/keydescription.h \
    models/wordcandidate.h \
    models/wordribbon.h \
    models/text.h \
    models/styleattributes.h \

SOURCES += \
    models/area.cpp \
    models/font.cpp \
    models/label.cpp \
    models/key.cpp \
    models/keyarea.cpp \
    models/keyareacontainer.cpp \
    models/wordcandidate.cpp \
    models/wordribbon.cpp \
    models/text.cpp \
    models/styleattributes.cpp \

DEPENDPATH += $$MODELS_DIR
