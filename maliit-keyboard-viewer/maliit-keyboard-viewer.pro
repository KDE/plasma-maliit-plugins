TEMPLATE = app
TARGET = maliit-keyboard-viewer
target.path = $$PREFIX/usr/bin

INCLUDEPATH += ../maliit-keyboard
LIBS += -L../maliit-keyboard -lmaliit-keyboard
SOURCES += main.cpp

QT += core gui
INSTALLS += target
