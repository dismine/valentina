#-------------------------------------------------
#
# Project created by QtCreator 2020-02-16T17:16:07
#
#-------------------------------------------------

# File with common stuff for whole project
include(../../../common.pri)

QT       += core gui widgets network xml svg xmlpatterns printsupport concurrent

# Name of binary file
TARGET = puzzle

VERSION = 0.7.52

# We want create executable file
TEMPLATE = app

# Use out-of-source builds (shadow builds)
CONFIG -= debug_and_release debug_and_release_target

# Since Q5.12 available support for C++17
equals(QT_MAJOR_VERSION, 5):greaterThan(QT_MINOR_VERSION, 11) {
    CONFIG += c++17
} else {
    CONFIG += c++14
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Since Qt 5.4.0 the source code location is recorded only in debug builds.
# We need this information also in release builds. For this need define QT_MESSAGELOGCONTEXT.
DEFINES += QT_MESSAGELOGCONTEXT

# Directory for executable file
DESTDIR = bin

# Directory for files created moc
MOC_DIR = moc

# Directory for objecs files
OBJECTS_DIR = obj

# Directory for files created rcc
RCC_DIR = rcc

# Directory for files created uic
UI_DIR = uic


# Suport subdirectories. Just better project code tree.
include(puzzle.pri)

RESOURCES += \
    share/resources/puzzlecursor.qrc \
    share/resources/puzzleicon.qrc

include(../svgfont.pri)
include(../translations.pri)

# Set "make install" command for Unix-like systems.
unix{
    !macx{
        # Path to bin file after installation
        target.path = $$BINDIR

        INSTALLS += \
            target
    }
    macx{
        # Some macx stuff
        QMAKE_MAC_SDK = macosx

        # QMAKE_MACOSX_DEPLOYMENT_TARGET defined in common.pri

        CONFIG(release, debug|release){
            QMAKE_RPATHDIR += @executable_path/../Frameworks

            # Path to resources in app bundle
            #RESOURCES_DIR = "Contents/Resources" defined in translation.pri
            FRAMEWORKS_DIR = "Contents/Frameworks"
            MACOS_DIR = "Contents/MacOS"
            # On macx we will use app bundle. Bundle doesn't need bin directory inside.
            # See issue #166: Creating OSX Homebrew (Mac OS X package manager) formula.
            target.path = $$MACOS_DIR

            #languages added inside translations.pri

            # Symlinks also good names for copying. Make will take origin file and copy them with using symlink name.
            # For bundle this names more then enough. We don't need care much about libraries versions.
            #libraries.path = $$FRAMEWORKS_DIR
            #libraries.files += $${OUT_PWD}/../../libs/qmuparser/$${DESTDIR}/libqmuparser.2.dylib
            #libraries.files += $${OUT_PWD}/../../libs/vpropertyexplorer/$${DESTDIR}/libvpropertyexplorer.1.dylib

            # logo on macx.
            ICON = $$PWD/../../../dist/Puzzle.icns

            QMAKE_INFO_PLIST = $$PWD/../../../dist/macx/puzzle/Info.plist

            format.path = $$RESOURCES_DIR/
            format.files += $$PWD/../../../dist/macx/layout.icns

            QMAKE_BUNDLE_DATA += \
                #libraries \
                format
        }
    }
}

#win32 {
#    for(DIR, INSTALL_OPENSSL) {
#        #add these absolute paths to a variable which
#        #ends up as 'mkcommands = path1 path2 path3 ...'
#        openssl_path += $${PWD}/$$DIR
#    }
#    copyToDestdir($$openssl_path, $$shell_path($${OUT_PWD}/$$DESTDIR))
#}

# Compilation will fail without this files after we added them to this section.
OTHER_FILES += \
    share/resources/puzzleicon/64x64/logo.ico  # Puzzle's logo.

# Set using ccache. Function enable_ccache() defined in common.pri.
$$enable_ccache()

include(warnings.pri)

CONFIG(release, debug|release){
    # Release mode
    !*msvc*:CONFIG += silent
    DEFINES += V_NO_ASSERT
    !unix:*g++*{
        QMAKE_CXXFLAGS += -fno-omit-frame-pointer # Need for exchndl.dll
    }

    noDebugSymbols{ # For enable run qmake with CONFIG+=noDebugSymbols
        DEFINES += V_NO_DEBUG
    } else {
        # Turn on debug symbols in release mode on Unix systems.
        # On Mac OS X temporarily disabled. Need find way how to strip binary file.
        !macx:!*msvc*{
            QMAKE_CXXFLAGS_RELEASE += -g -gdwarf-3
            QMAKE_CFLAGS_RELEASE += -g -gdwarf-3
            QMAKE_LFLAGS_RELEASE =
        }
    }
} else {
# Breakpoints do not work if debug the app inside of bundle. In debug mode we turn off creating a bundle.
# Probably it will breake some dependencies. Version for Mac designed to work inside an app bundle.
    CONFIG -= app_bundle
}

DVCS_HESH=$$FindBuildRevision()
message("Build revision:" $${DVCS_HESH})
DEFINES += "BUILD_REVISION=$${DVCS_HESH}" # Make available build revision number in sources.

# Path to recource file.
win32:RC_FILE = share/resources/puzzle.rc

noRunPath{ # For enable run qmake with CONFIG+=noRunPath
    # do nothing
} else {
    unix:!macx{
        # suppress the default RPATH
        # helps to run the program without Qt Creator
        # see problem with path to libqmuparser and libpropertybrowser
        QMAKE_LFLAGS_RPATH =
        QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\' -Wl,-rpath,$${OUT_PWD}/../../libs/qmuparser/$${DESTDIR} -Wl,-rpath,$${OUT_PWD}/../../libs/vpropertyexplorer/$${DESTDIR}"
    }
}

# When the GNU linker sees a library, it discards all symbols that it doesn't need.
# Dependent library go first.

##VTools static library (depend on VWidgets, VMisc, VPatternDB)
#unix|win32: LIBS += -L$$OUT_PWD/../../libs/vtools/$${DESTDIR}/ -lvtools

#INCLUDEPATH += $$PWD/../../libs/vtools
#INCLUDEPATH += $$OUT_PWD/../../libs/vtools/$${UI_DIR} # For UI files
#DEPENDPATH += $$PWD/../../libs/vtools

#win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vtools/$${DESTDIR}/vtools.lib
#else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vtools/$${DESTDIR}/libvtools.a

# VLayout static library (depend on VGeometry, VFormat, VWidgets, VFormat)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vlayout/$${DESTDIR}/ -lvlayout

INCLUDEPATH += $$PWD/../../libs/vlayout
DEPENDPATH += $$PWD/../../libs/vlayout

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vlayout/$${DESTDIR}/vlayout.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vlayout/$${DESTDIR}/libvlayout.a

#VWidgets static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vwidgets/$${DESTDIR}/ -lvwidgets

INCLUDEPATH += $$PWD/../../libs/vwidgets
DEPENDPATH += $$PWD/../../libs/vwidgets

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vwidgets/$${DESTDIR}/vwidgets.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vwidgets/$${DESTDIR}/libvwidgets.a

# VFormat static library (depend on VPatternDB, IFC)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vformat/$${DESTDIR}/ -lvformat

INCLUDEPATH += $$PWD/../../libs/vformat
DEPENDPATH += $$PWD/../../libs/vformat

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vformat/$${DESTDIR}/vformat.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vformat/$${DESTDIR}/libvformat.a

#VPatternDB static library (depend on vgeometry, vmisc)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vpatterndb/$${DESTDIR} -lvpatterndb

INCLUDEPATH += $$PWD/../../libs/vpatterndb
DEPENDPATH += $$PWD/../../libs/vpatterndb

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vpatterndb/$${DESTDIR}/vpatterndb.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vpatterndb/$${DESTDIR}/libvpatterndb.a

# Fervor static library (depend on VMisc, IFC)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/fervor/$${DESTDIR}/ -lfervor

INCLUDEPATH += $$PWD/../../libs/fervor
DEPENDPATH += $$PWD/../../libs/fervor

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/fervor/$${DESTDIR}/fervor.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/fervor/$${DESTDIR}/libfervor.a

# IFC static library (depend on QMuParser, VMisc)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/ifc/$${DESTDIR}/ -lifc

INCLUDEPATH += $$PWD/../../libs/ifc
DEPENDPATH += $$PWD/../../libs/ifc

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/ifc/$${DESTDIR}/ifc.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/ifc/$${DESTDIR}/libifc.a

#VMisc static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vmisc/$${DESTDIR}/ -lvmisc

INCLUDEPATH += $$PWD/../../libs/vmisc
DEPENDPATH += $$PWD/../../libs/vmisc

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vmisc/$${DESTDIR}/vmisc.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vmisc/$${DESTDIR}/libvmisc.a

# VGeometry static library (depend on ifc)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vgeometry/$${DESTDIR}/ -lvgeometry

INCLUDEPATH += $$PWD/../../libs/vgeometry
DEPENDPATH += $$PWD/../../libs/vgeometry

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vgeometry/$${DESTDIR}/vgeometry.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vgeometry/$${DESTDIR}/libvgeometry.a

# VObj static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vobj/$${DESTDIR}/ -lvobj

INCLUDEPATH += $$PWD/../../libs/vobj
DEPENDPATH += $$PWD/../../libs/vobj

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vobj/$${DESTDIR}/vobj.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vobj/$${DESTDIR}/libvobj.a

# VDxf static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vdxf/$${DESTDIR}/ -lvdxf

INCLUDEPATH += $$PWD/../../libs/vdxf
DEPENDPATH += $$PWD/../../libs/vdxf

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vdxf/$${DESTDIR}/vdxf.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vdxf/$${DESTDIR}/libvdxf.a

# VHPGL static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vhpgl/$${DESTDIR}/ -lvhpgl

INCLUDEPATH += $$PWD/../../libs/vhpgl
DEPENDPATH += $$PWD/../../libs/vhpgl

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vhpgl/$${DESTDIR}/vhpgl.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vhpgl/$${DESTDIR}/libvhpgl.a

# VGAnalytics static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vganalytics/$${DESTDIR}/ -lvganalytics

INCLUDEPATH += $$PWD/../../libs/vganalytics
DEPENDPATH += $$PWD/../../libs/vganalytics

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vganalytics/$${DESTDIR}/vganalytics.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vganalytics/$${DESTDIR}/libvganalytics.a

# QMuParser library
win32:CONFIG(release, debug|release): LIBS += -L$${OUT_PWD}/../../libs/qmuparser/$${DESTDIR} -lqmuparser2
else:win32:CONFIG(debug, debug|release): LIBS += -L$${OUT_PWD}/../../libs/qmuparser/$${DESTDIR} -lqmuparser2
else:unix: LIBS += -L$${OUT_PWD}/../../libs/qmuparser/$${DESTDIR} -lqmuparser

INCLUDEPATH += $${PWD}/../../libs/qmuparser
DEPENDPATH += $${PWD}/../../libs/qmuparser

# VPropertyExplorer library
win32:CONFIG(release, debug|release): LIBS += -L$${OUT_PWD}/../../libs/vpropertyexplorer/$${DESTDIR} -lvpropertyexplorer
else:win32:CONFIG(debug, debug|release): LIBS += -L$${OUT_PWD}/../../libs/vpropertyexplorer/$${DESTDIR} -lvpropertyexplorer
else:unix: LIBS += -L$${OUT_PWD}/../../libs/vpropertyexplorer/$${DESTDIR} -lvpropertyexplorer

INCLUDEPATH += $${PWD}/../../libs/vpropertyexplorer
DEPENDPATH += $${PWD}/../../libs/vpropertyexplorer

contains(DEFINES, APPIMAGE) {
    unix:!macx: LIBS += -licudata -licui18n -licuuc
}

CONFIG(release, debug|release){
    noStripDebugSymbols {
        # do nothing
    } else {
        !macx:!*msvc*{
            noDebugSymbols{ # For enable run qmake with CONFIG+=noDebugSymbols
                win32:!*msvc*{
                    # Strip debug symbols.
                    QMAKE_POST_LINK += objcopy --strip-debug bin/${TARGET}
                }

                unix:!macx{
                    # Strip after you link all libaries.
                    QMAKE_POST_LINK += objcopy --strip-debug ${TARGET}
                }
            } else {
                win32:!*msvc*{
                    # Strip debug symbols.
                    QMAKE_POST_LINK += objcopy --only-keep-debug bin/${TARGET} bin/${TARGET}.dbg &&
                    QMAKE_POST_LINK += objcopy --strip-debug bin/${TARGET} &&
                    QMAKE_POST_LINK += objcopy --add-gnu-debuglink="bin/${TARGET}.dbg" bin/${TARGET}

                    QMAKE_DISTCLEAN += bin/${TARGET}.dbg
                }

                unix:!macx{
                    # Strip debug symbols.
                    QMAKE_POST_LINK += objcopy --only-keep-debug ${TARGET} ${TARGET}.dbg &&
                    QMAKE_POST_LINK += objcopy --strip-debug ${TARGET} &&
                    QMAKE_POST_LINK += objcopy --add-gnu-debuglink="${TARGET}.dbg" ${TARGET}

                    QMAKE_DISTCLEAN += ${TARGET}.dbg
                }
            }
        }
    }
}

CONFIG(release, debug|release){
    macx{
       # run macdeployqt to include all qt libraries in packet
       QMAKE_POST_LINK += $$[QT_INSTALL_BINS]/macdeployqt $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app
    }
}
