CONFIG += ordered
TEMPLATE = subdirs
SUBDIRS = \
          ut_buttonbar \
          ut_duivirtualkeyboard \
          ut_duikeyboardplugin \
          ut_duiimcorrectioncandidatewidget \
          ut_keybutton \
          ut_keybuttonarea \
          ut_symbolview \
          bm_keybuttonarea \
          bm_symbols \
          ut_duikeyboardhost \
          ut_notification \
          ut_limitedtimer \
          ut_vkbdatakey \
          ut_horizontalswitcher \
          ut_layoutsmanager \
          ut_duiimtoolbar \
          ut_toolbarmanager \
          ut_duihardwarekeyboard \
          ut_hwkbcharloopsmanager \

target.commands += $$system(touch tests.xml)
target.path = /usr/share/dui-im-virtualkeyboard-tests
target.files += qtestlib2junitxml.xsl runtests.sh tests.xml
INSTALLS += target

QMAKE_EXTRA_TARGETS += check-xml
check-xml.target = check-xml
check-xml.CONFIG = recursive

QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive

