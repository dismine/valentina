import qbs.FileInfo

VApp {
    Depends { name: "buildconfig" }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "freedesktop" }
    Depends { name: "Qt"; submodules: ["widgets", "svg"] }
    Depends { name: "VMiscLib"; }
    Depends { name: "VPatternDBLib"; }
    Depends { name: "FervorLib"; }
    Depends { name: "QMUParserLib"; }
    Depends { name: "VFormatLib"; }
    Depends { name: "VWidgetsLib"; }
    Depends { name: "VToolsLib"; }

    Properties {
        condition: cpp.defines.contains("APPIMAGE")
        cpp.dynamicLibraries: ["icudata", "icui18n", "icuuc"]
    }

    name: "Tape"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "Tape" : "tape"
    targetName: buildconfig.appTarget

    files: [
        "main.cpp",
        "tmainwindow.cpp",
        "mapplication.cpp",
        "vlitepattern.cpp",
        "vtapesettings.cpp",
        "tmainwindow.h",
        "mapplication.h",
        "version.h",
        "vlitepattern.h",
        "vtapesettings.h",
        "tmainwindow.ui",
    ]

    Group {
        name: "dialogs"
        prefix: "dialogs/"
        files: [
            "dialogdimensioncustomnames.cpp",
            "dialogdimensionlabels.cpp",
            "dialogmeasurementscsvcolumns.cpp",
            "dialogrestrictdimension.cpp",
            "dialogabouttape.cpp",
            "dialognewmeasurements.cpp",
            "dialogmdatabase.cpp",
            "dialogtapepreferences.cpp",
            "configpages/tapepreferencesconfigurationpage.cpp",
            "configpages/tapepreferencespathpage.cpp",
            "dialogsetupmultisize.cpp",
            "dialogdimensioncustomnames.h",
            "dialogdimensionlabels.h",
            "dialogmeasurementscsvcolumns.h",
            "dialogrestrictdimension.h",
            "dialogabouttape.h",
            "dialognewmeasurements.h",
            "dialogmdatabase.h",
            "dialogtapepreferences.h",
            "configpages/tapepreferencesconfigurationpage.h",
            "configpages/tapepreferencespathpage.h",
            "dialogsetupmultisize.h",
            "dialogdimensioncustomnames.ui",
            "dialogdimensionlabels.ui",
            "dialogmeasurementscsvcolumns.ui",
            "dialogrestrictdimension.ui",
            "dialogabouttape.ui",
            "dialognewmeasurements.ui",
            "dialogmdatabase.ui",
            "dialogtapepreferences.ui",
            "configpages/tapepreferencesconfigurationpage.ui",
            "configpages/tapepreferencespathpage.ui",
            "dialogsetupmultisize.ui",
        ]
    }

    Group {
        name: "Resources"
        files: [
            "share/resources/tapeicon.qrc",
        ]
    }

    Group {
        name: "Precompiled headers"
        files: {
            var files = ["stable.h"];
            if (qbs.toolchain.contains("msvc"))
                files.push("stable.cpp")
            return files;
        }
        fileTags: ["cpp_pch_src"]
    }

    Export {
        Depends { name: "cpp" }
        cpp.defines: {
            var defines = [];
            var extension = qbs.targetOS.contains("windows") ? ".exe" : "";
            defines.push('TAPE_BUILDDIR="' + exportingProduct.buildDirectory + FileInfo.pathSeparator() +
                         exportingProduct.targetName + extension +'"');
            return defines;
        }
    }
}
