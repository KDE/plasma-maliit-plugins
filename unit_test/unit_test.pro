CONFIG += ordered
TEMPLATE = subdirs
SUBDIRS = \
          ut_buttonbar \
          ut_mvirtualkeyboard \
          ut_mkeyboardplugin \
          ut_mimcorrectioncandidatewidget \
          ut_keybutton \
          ut_keybuttonarea \
          ut_symbolview \
          bm_keybuttonarea \
          bm_symbols \
          ut_mkeyboardhost \
          ut_notification \
          ut_limitedtimer \
          ut_vkbdatakey \
          ut_horizontalswitcher \
          ut_layoutsmanager \
          ut_mimtoolbar \
          ut_toolbarmanager \
          ut_mhardwarekeyboard \
          ut_hwkbcharloopsmanager \

target.commands += $$system(touch tests.xml)
target.path = /usr/share/m-keyboard-tests
target.files += qtestlib2junitxml.xsl runtests.sh tests.xml
INSTALLS += target

QMAKE_EXTRA_TARGETS += check-xml
check-xml.target = check-xml
check-xml.CONFIG = recursive

QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive

