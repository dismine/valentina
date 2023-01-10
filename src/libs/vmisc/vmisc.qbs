VLib {
    Depends { name: "Qt"; submodules: ["printsupport"] }

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
        "diagnostic.h",
        "customevents.h",
        "defglobal.h",
        "testvapplication.h",
        "literals.h",
        "qt_dispatch/qt_dispatch.h",
        "vdatastreamenum.h",
        "vmodifierkey.h",
        "typedef.h",
    ]

    Group {
        name: "AppImage"
        condition: cpp.defines.contains("APPIMAGE")
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
            "dialogexporttocsv.cpp",
            "dialogselectlanguage.cpp",
            "dialogexporttocsv.h",
            "dialogselectlanguage.h",
            "dialogexporttocsv.ui",
            "dialogselectlanguage.ui"
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
        name: "Resources"
        prefix: "share/resources/"
        files: [
            "theme.qrc", // Windows theme icons.
            "icon.qrc", // All other icons except cursors and Windows theme.
            "flags.qrc",
            "qdarkstyle/style.qrc"
        ]
    }

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["printsupport"] }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
