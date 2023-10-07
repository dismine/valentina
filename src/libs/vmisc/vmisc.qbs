import qbs.Utilities

VLib {
    Depends { name: "Qt"; submodules: ["core", "printsupport", "gui"] }

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
            "vdatastreamenum.h",
            "vmodifierkey.h",
            "typedef.h",
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
            "binreloc.h",
            "appimage.h",
            "binreloc.c",
            "appimage.cpp"
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
        ]
    }

    Group {
        name: "bpstd"
        prefix: "bpstd/"
        files: [
            "any.hpp",
            "chrono.hpp",
            "complex.hpp",
            "cstddef.hpp",
            "detail/config.hpp",
            "detail/enable_overload.hpp",
            "detail/invoke.hpp",
            "detail/move.hpp",
            "detail/nth_type.hpp",
            "detail/proxy_iterator.hpp",
            "detail/variant_base.hpp",
            "detail/variant_fwds.hpp",
            "detail/variant_traits.hpp",
            "detail/variant_union.hpp",
            "detail/variant_visitors.hpp",
            "exception.hpp",
            "functional.hpp",
            "iterator.hpp",
            "memory.hpp",
            "optional.hpp",
            "span.hpp",
            "string.hpp",
            "string_view.hpp",
            "tuple.hpp",
            "type_traits.hpp",
            "utility.hpp",
            "variant.hpp",
        ]
    }

    Group {
        name: "backport"
        prefix: "backport/"
        files: [
            "qoverload.h",
            "qscopeguard.h",
            "text.h",
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

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["printsupport"] }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
