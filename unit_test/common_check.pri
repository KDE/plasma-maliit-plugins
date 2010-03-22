QMAKE_EXTRA_TARGETS += check
check.target = check
check.commands = LD_LIBRARY_PATH=../../dui-keyboard/:$(LD_LIBRARY_PATH) ./$$TARGET
check.depends += $$TARGET

QMAKE_EXTRA_TARGETS += check-xml
check-xml.target = check-xml
check-xml.commands = ../rt.sh $$TARGET
check-xml.depends += $$TARGET

QMAKE_CLEAN += *.log *.xml *~

target.path = /usr/lib/dui-im-virtualkeyboard-tests/$$TARGET
INSTALLS += target

QMAKE_CXXFLAGS += -Werror

DEFINES += UNIT_TEST

QT += testlib

DUIKEYBOARD_DIR = ../../dui-keyboard
WIDGETS_DIR = $$DUIKEYBOARD_DIR/widgets
COMMON_DIR = $$DUIKEYBOARD_DIR/common

INCLUDEPATH += \
    $$WIDGETS_DIR \
    $$COMMON_DIR \
    $$DUIKEYBOARD_DIR \

LIBS += \
    -L $$DUIKEYBOARD_DIR \

# coverage flags are off per default, but can be turned on via qmake COV_OPTION=on
for(OPTION,$$list($$lower($$COV_OPTION))){
    isEqual(OPTION, on){
	QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs
	LIBS += -lgcov
    }
}

QMAKE_CLEAN += *.gcno *.gcda

CONFIG += duiimframework
