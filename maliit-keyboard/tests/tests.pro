TEMPLATE = subdirs
SUBDIRS = \
    common \
    language-layout-switching \
    preedit-string \
    repeat-backspace \
    rendering \

CONFIG += ordered
QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive
