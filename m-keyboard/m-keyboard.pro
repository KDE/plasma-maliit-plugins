TEMPLATE = lib 
TARGET = $$qtLibraryTarget(mkeyboard)

OBJECTS_DIR = .obj
MOC_DIR = .moc

QMAKE_CXXFLAGS += -Werror

# we have this line temporarily until new libmeegotouch without rpath is integrated
QT += xml

CONFIG += plugin meegotouch duiimengine duiimenginewords mimframework duireactionmap
#CONFIG += mcontrolpanel
DEFINES += NOCONTROLPANEL

CONFIG += link_pkgconfig
PKGCONFIG += gconf-2.0

# coverage flags are off per default, but can be turned on via qmake COV_OPTION=on
for(OPTION,$$list($$lower($$COV_OPTION))){
    isEqual(OPTION, on){
        QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs -fno-elide-constructors
        LIBS += -lgcov
        CONFIG -= release
        CONFIG += debug
    }
}

QMAKE_CLEAN += *.gcno *.gcda

include(common/common.pri)
include(widgets/widgets.pri)
include(theme/theme.pri)
include(layouts/layouts.pri)

HEADERS += \
    mkeyboardhost.h \
    mkeyboardplugin.h \

SOURCES += \
    mkeyboardhost.cpp \
    mkeyboardplugin.cpp \

target.path += /usr/lib/m-im-plugins

install_headers.path = /usr/include/meego-keyboard
install_headers.files = $$INSTALL_HEADERS


INSTALLS += \
    target \
    install_headers \

QMAKE_EXTRA_TARGETS += check-xml
check-xml.depends = lib$${TARGET}.so
check-xml.commands = $$system(true)

QMAKE_EXTRA_TARGETS += check
check.depends = lib$${TARGET}.so
check.commands = $$system(true)

