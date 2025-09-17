import qbs.Utilities

VLib {
    Depends { name: "Qt"; submodules: ["core", "printsupport", "gui", "widgets"] }
    Depends { name: "buildconfig" }

    Depends {
        name: "crashpad";
        condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled
    }

    Depends {
        name: "icu";
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 &&
                   project.withTextCodec && project.withICUCodecs &&
                   buildconfig.useConanPackages && buildconfig.conanWithICUEnabled
    }

    Depends {
        name: "libiconv";
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 &&
                   project.withTextCodec && !project.withICUCodecs && project.withICONVCodecs &&
                   buildconfig.useConanPackages && buildconfig.conanWithICONVEnabled
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

    Properties {
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 &&
                   (qbs.targetOS.contains("darwin") ||
                    qbs.targetOS.contains("bsd") ||
                    qbs.targetOS.contains("windows")) &&
                   project.withTextCodec && !project.withICUCodecs && project.withICONVCodecs &&
                   (!buildconfig.useConanPackages ||
                   (buildconfig.useConanPackages && !buildconfig.conanWithICONVEnabled))
        cpp.dynamicLibraries: ["iconv"]
    }

    Properties {
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 &&
                   project.withTextCodec && project.withICUCodecs  &&
                   (!buildconfig.useConanPackages ||
                   (buildconfig.useConanPackages && !buildconfig.conanWithICUEnabled))
        cpp.dynamicLibraries: ["icuuc", "icui18n", "icudata"]
    }

    name: "VMiscLib"
    files: [
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

    Group {
        name: "VTextStream"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0
        files: [
            "vtextstream.h",
            "vtextstream.cpp",
            "vtextstream_p.h"
        ]
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
            "dialogselectmeasurementstype.cpp",
            "dialogselectmeasurementstype.h",
            "dialogselectmeasurementstype.ui",
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
            "vsvghandler.cpp",
            "vsvghandler.h",
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
        name: "Codecs: Text codec"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 && project.withTextCodec
        prefix: "codecs/"
        files: [
            "qstringiterator_p.h",
            "qtextcodec.h",
            "qtextcodec.cpp",
            "qtextcodec_p.h",
            "qutfcodec.cpp",
            "qutfcodec_p.h",
            "qcodecmacros_p.h",
            "qsimplecodec.cpp",
            "qsimplecodec_p.h",
            "qlatincodec.cpp",
            "qlatincodec_p.h",
        ]
    }

    Group {
        name: "Codecs: Text codec (internal)"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 && !project.withTextCodec
        files: [
            "vtextcodec.cpp",
            "vtextcodec.h"
        ]
    }

    Group {
        name: "Codecs: Basic codecs"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 && project.withTextCodec && project.withBasicCodecs
        prefix: "codecs/"
        files: [
            "qtsciicodec.cpp",
            "qtsciicodec_p.h",
            "qisciicodec.cpp",
            "qisciicodec_p.h",
        ]
    }

    Group {
        name: "Codecs: ICU codecs"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 && project.withTextCodec && project.withICUCodecs
        prefix: "codecs/"
        files: [
            "qicucodec.cpp",
            "qicucodec_p.h",
        ]
    }

    Group {
        name: "Codecs: ICONV codecs"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 && project.withTextCodec && !project.withICUCodecs && project.withICONVCodecs
        prefix: "codecs/"
        files: [
            "qiconvcodec.cpp",
            "qiconvcodec_p.h",
        ]
    }

    Group {
        name: "Codecs: Windows codecs"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 && project.withTextCodec && !project.withICUCodecs && qbs.targetOS.contains("windows")
        prefix: "codecs/"
        files: [
            "qwindowscodec.cpp",
            "qwindowscodec_p.h",
        ]
    }

    Group {
        name: "Codecs: Big codecs"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 && project.withTextCodec && !project.withICUCodecs && project.withBigCodecs
        prefix: "codecs/"
        files: [
            "cp949codetbl_p.h",
            "qbig5codec.cpp",
            "qbig5codec_p.h",
            "qeucjpcodec.cpp",
            "qeucjpcodec_p.h",
            "qeuckrcodec.cpp",
            "qeuckrcodec_p.h",
            "qgb18030codec.cpp",
            "qgb18030codec_p.h",
            "qjiscodec.cpp",
            "qjiscodec_p.h",
            "qjpunicode.cpp",
            "qjpunicode_p.h",
            "qsjiscodec.cpp",
            "qsjiscodec_p.h",
        ]
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

    Group {
        name: "backport"
        prefix: "backport/"
        files: [
            "qpainterstateguard.h",
            "qpainterstateguard.cpp",
        ]
        condition: Utilities.versionCompare(Qt.core.version, "6.9") < 0
    }

    Group {
        name: "exception"
        prefix: "exception/"
        files: [
            "vexception.h",
            "vexception.cpp"
        ]
    }

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["printsupport", "widgets"] }
        Depends {
            name: "crashpad";
            condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled
        }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
