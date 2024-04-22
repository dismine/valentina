import qbs.Utilities

VLib {
    Depends { name: "Qt"; submodules: ["core", "printsupport", "gui", "widgets"] }
    Depends { name: "buildconfig" }

    Depends {
        name: "conan.crashpad";
        condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled
    }

    Properties {
        condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled && qbs.targetOS.contains("windows") && qbs.toolchain.contains("msvc")
        cpp.dynamicLibraries: ["Advapi32"]
    }

    Properties {
        condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled && qbs.targetOS.contains("macos")
        cpp.libraryPaths: ["/usr/lib"]
        cpp.dynamicLibraries: ["bsm"]
        cpp.frameworks: ["AppKit", "Security"]
    }

    Properties {
        condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled && qbs.targetOS.contains("unix") && !qbs.targetOS.contains("macos")
        cpp.dynamicLibraries: ["dl"]
    }

    name: "VMiscLib"
    files: {
        var files = [
            "def.cpp",
            "testpath.cpp",
            "vabstractvalapplication.cpp",
            "vabstractapplication.cpp",
            "projectversion.cpp",
            "vcommonsettings.cpp",
            "vvalentinasettings.cpp",
            "commandoptions.cpp",
            "qxtcsvmodel.cpp",
            "vtablesearch.cpp",
            "literals.cpp",
            "vmodifierkey.cpp",
            "compatibility.h",
            "lambdaconstants.h",
            "def.h",
            "testpath.h",
            "vabstractvalapplication.h",
            "vmath.h",
            "vabstractapplication.h",
            "projectversion.h",
            "vcommonsettings.h",
            "vvalentinasettings.h",
            "debugbreak.h",
            "vlockguard.h",
            "vsysexits.h",
            "commandoptions.h",
            "qxtcsvmodel.h",
            "vtablesearch.h",
            "customevents.h",
            "defglobal.h",
            "testvapplication.h",
            "literals.h",
            "qt_dispatch/qt_dispatch.h",
            "vmodifierkey.h",
            "typedef.h",
            "vabstractshortcutmanager.h",
            "vabstractshortcutmanager.cpp",
            "vtranslator.h",
            "vtranslator.cpp",
        ]

        if (Utilities.versionCompare(Qt.core.version, "6") >= 0) {
            files.push("vtextcodec.cpp", "vtextcodec.h");
        }

        return files;
    }

    Group {
        name: "AppImage"
        condition: buildconfig.enableAppImage && qbs.targetOS.contains("unix") && !qbs.targetOS.contains("macos")
        files: [
            "appimage.h",
            "appimage.cpp",
        ]
    }

    Group {
        name: "dialogs"
        prefix: "dialogs/"
        files: [
            "dialogaskcollectstatistic.cpp",
            "dialogaskcollectstatistic.h",
            "dialogaskcollectstatistic.ui",
            "dialogexporttocsv.cpp",
            "dialogselectlanguage.cpp",
            "dialogexporttocsv.h",
            "dialogselectlanguage.h",
            "dialogexporttocsv.ui",
            "dialogselectlanguage.ui",
            "vshortcutdialog.cpp",
            "vshortcutdialog.h",
            "vshortcutdialog.ui",
        ]
    }

    Group {
        name: "fpm"
        prefix: "fpm/"
        files: [
            "fixed.hpp",
            "math.hpp",
        ]
    }

    Group {
        name: "svgfont"
        prefix: "svgfont/"
        files: [
            "svgdef.cpp",
            "svgdef.h",
            "vsvgfont.cpp",
            "vsvgfont.h",
            "vsvgfont_p.h",
            "vsvgfontdatabase.cpp",
            "vsvgfontdatabase.h",
            "vsvgfontengine.cpp",
            "vsvgfontengine.h",
            "vsvgfontengine_p.h",
            "vsvgfontreader.cpp",
            "vsvgfontreader.h",
            "vsvgglyph.cpp",
            "vsvgglyph.h",
            "vsvgglyph_p.h",
            "vsvgpathtokenizer.cpp",
            "vsvgpathtokenizer.h",
        ]
    }

    Group {
        name: "theme"
        prefix: "theme/"
        files: [
            "themeDef.cpp",
            "themeDef.h",
            "vapplicationstyle.cpp",
            "vapplicationstyle.h",
            "vmanuallayoutstyle.cpp",
            "vmanuallayoutstyle.h",
            "vpatternpiecestyle.cpp",
            "vpatternpiecestyle.h",
            "vscenestylesheet.cpp",
            "vscenestylesheet.h",
            "vstylesheetstyle.cpp",
            "vstylesheetstyle.h",
            "vtheme.h",
            "vtheme.cpp",
            "vtoolstyle.cpp",
            "vtoolstyle.h",
        ]
    }

    Group {
        name: "Mac utils"
        prefix: "theme/"
        files: [
            "macutils.h",
            "macutils.mm"
        ]
        condition: qbs.targetOS.contains("macos")
    }

    Group {
        name: "Resources"
        prefix: "share/resources/"
        files: [
            "icon.qrc",
            "flags.qrc",
            "breezethemes/breeze.qrc",
            "scenestyle.qrc",
            "cursor.qrc",
        ]
    }

    Group {
        name: "Win icon themes"
        prefix: "share/resources/"
        files: [
            "win_light_theme.qrc",
            "win_dark_theme.qrc"
        ]
    }

    Group {
        name: "Mac icon themes"
        prefix: "share/resources/"
        files: [
            "mac_light_theme.qrc",
            "mac_dark_theme.qrc"
        ]
        condition: qbs.targetOS.contains("macos")
    }

    Group {
        name: "crashhandler"
        prefix: "crashhandler/"
        files: [
            "crashhandler.h",
            "crashhandler.cpp",
            "vcrashpaths.cpp",
            "vcrashpaths.h",
        ]
        condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled
    }

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["printsupport", "widgets"] }
        Depends {
            name: "conan.crashpad";
            condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled
        }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
