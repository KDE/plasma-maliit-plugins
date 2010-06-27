DOXYGEN_BIN=doxygen

QMAKE_EXTRA_TARGETS += doc
doc.target = doc

isEmpty(DOXYGEN_BIN) {
    doc.commands = @echo "Unable to detect doxygen in PATH"
} else {
  doc.commands = mkdir -p $${OUT_PWD}/doc/html/ ;
  doc.commands += ( cat $${IN_PWD}/mdoxy.cfg.in | \
      perl -pe \"s:\\@M_SRC_DIR\\@:$${IN_PWD}:\" > $${OUT_PWD}/doc/mdoxy.cfg );

  doc.commands+= ( cd doc ; $${DOXYGEN_BIN} mdoxy.cfg );
  doc.commands+= ( cd doc ; $${IN_PWD}/xmlize.pl );

  # Install rules
  htmldocs.files = $${OUT_PWD}/doc/html

  htmldocs.path = /usr/share/doc/meego-im-framework
  htmldocs.CONFIG += no_check_exist
  INSTALLS += htmldocs

  # TODO: how to remove the html directory?
  QMAKE_CLEAN += $${OUT_PWD}/doc/mdoxy.cfg $${OUT_PWD}/doc/doxygen.log $${OUT_PWD}/doc/doxygen.log.xml
}

doc.depends = FORCE
