include(common.pri)

#Check if Qt version >= 5.6.0
!minQtVersion(5, 6, 0) {
    message("Cannot build Valentina with Qt version $${QT_VERSION}.")
    error("Use at least Qt 5.6.0.")
}

#These checks need because we can quote all paths that contain space.
LIST = $$split(PWD,' ')
count(LIST, 1, >): error("The build will fail. Path '$${PWD}' contains space!!!")

LIST = $$split(OUT_PWD,' ')
count(LIST, 1, >): error("The build will fail. Path '$${OUT_PWD}' contains space!!!")

TEMPLATE = subdirs
SUBDIRS = src
