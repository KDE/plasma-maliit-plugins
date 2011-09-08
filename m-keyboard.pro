include(config.pri)

!isEmpty(HELP) {
    # Output help
    help_string = \
        Important build options: \
        \\n\\t \
        \\nRecognised CONFIG flags: \
        \\n\\t enable-legacy : Build for legacy mode (using meego-im ABI/API compatabible maliit-framework) \
        \\n\\t enable-meegotouch : Build libmeegotouch based keyboard plugin \
        \\n\\t notests : Do not build tests \
        \\nInfluential environment variables: \
        \\n\\t PKG_CONFIG_PATH: Specify the pkg-config files to use for dependencies \
        \\nExamples: \
        \\n\\t qmake \
        \\n\\t qmake CONFIG+=enable-legacy CONFIG+=notests \

    !build_pass:system(echo \"$$help_string\")
} else {
    config_string = Tip: Run qmake HELP=1 for a list of all supported build options

    !build_pass:system(echo \"$$config_string\")
}

CONFIG += ordered 
TARGET = meego-im-plugins
TEMPLATE = subdirs
SUBDIRS = \
    meego-keyboard-quick \

CONFIG(docs) {
    include (doc/doc.pri)
}

!nomeegotouch {
    SUBDIRS += \
        m-keyboard \
        translations \

}

!notests:!nomeegotouch {
    SUBDIRS += \
        tests \
        fixture_virtualkeyboard \

}

!nomeegotouch {
    #error-correction
    GCONF_DATA = meego-keyboard.schemas
    isEmpty(GCONF_SCHEMAS_DIR) {
        GCONF_SCHEMAS_DIR = /usr/share/gconf/schemas
    }
    gconf_data.path = $$GCONF_SCHEMAS_DIR
    gconf_data.files = $$GCONF_DATA

    INSTALLS += gconf_data \
}

QMAKE_EXTRA_TARGETS += check-xml
check-xml.target = check-xml
check-xml.CONFIG = recursive

QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive

QMAKE_CLEAN += build-stamp configure-stamp

