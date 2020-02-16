TEMPLATE = subdirs
SUBDIRS = \
    tape \
    valentina \
    puzzle

macx{# For making app bundle tape and puzzle must exist before valentina.app will be created
    valentina.depends = tape
    valentina.depends = puzzle
}
