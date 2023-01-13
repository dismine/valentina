import qbs.FileInfo

VToolApp {
    Depends { name: "buildconfig" }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "Qt"; submodules: ["widgets", "svg"] }
    Depends { name: "VMiscLib"; }
    Depends { name: "VPatternDBLib"; }
    Depends { name: "FervorLib"; }
    Depends { name: "QMUParserLib"; }
    Depends { name: "VFormatLib"; }
    Depends { name: "VWidgetsLib"; }
    Depends { name: "VToolsLib"; }
    Depends { name: "ebr" }

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

    Group {
        name: "Multisize tables"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "src", "app", "share", "tables", "multisize", FileInfo.pathSeparator())
        files: [
            "GOST_man_ru.vst"
        ]
        fileTags: ["multisize_tables"]
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + FileInfo.pathSeparator() + "tables" + FileInfo.pathSeparator() + "multisize"
    }

    Group {
        name: "Measurements templates"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "src", "app", "share", "tables", "templates", FileInfo.pathSeparator())
        files: [
            "template_all_measurements.vit",
            "t_Aldrich_Women.vit"
        ]
        fileTags: ["measurements_templates"]
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + FileInfo.pathSeparator() + "tables" + FileInfo.pathSeparator() + "templates"
    }

    Group {
        name: "48x48/apps"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "48x48", "apps", FileInfo.pathSeparator())
        files: "tape.png"
        fileTags: "freedesktop.48x48AppsIcons"
    }

    Group {
        name: "64x64/apps"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "64x64", "apps", FileInfo.pathSeparator())
        files: "tape.png"
        fileTags: "freedesktop.64x64AppsIcons"
    }

    Group {
        name: "128x128/apps"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "128x128", "apps", FileInfo.pathSeparator())
        files: "tape.png"
        fileTags: "freedesktop.128x128AppsIcons"
    }

    Group {
        name: "256x256/apps"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "256x256", "apps", FileInfo.pathSeparator())
        files: "tape.png"
        fileTags: "freedesktop.256x256AppsIcons"
    }

    Group {
        name: "512x512/apps"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "512x512", "apps", FileInfo.pathSeparator())
        files: "tape.png"
        fileTags: "freedesktop.512x512AppsIcons"
    }

    Group {
        name: "apps48x48/mimetypes"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "48x48", "mimetypes", FileInfo.pathSeparator())
        files: ["application-x-valentina-i-measurements.png", "application-x-valentina-s-measurements.png"]
        fileTags: "freedesktop.48x48MimetypesIcons"
    }

    Group {
        name: "apps64x64/mimetypes"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "64x64", "mimetypes", FileInfo.pathSeparator())
        files: ["application-x-valentina-i-measurements.png", "application-x-valentina-s-measurements.png"]
        fileTags: "freedesktop.64x64MimetypesIcons"
    }

    Group {
        name: "apps128x128/mimetypes"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "128x128", "mimetypes", FileInfo.pathSeparator())
        files: ["application-x-valentina-i-measurements.png", "application-x-valentina-s-measurements.png"]
        fileTags: "freedesktop.128x128MimetypesIcons"
    }

    Group {
        name: "apps256x256/mimetypes"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "256x256", "mimetypes", FileInfo.pathSeparator())
        files: ["application-x-valentina-i-measurements.png", "application-x-valentina-s-measurements.png"]
        fileTags: "freedesktop.256x256MimetypesIcons"
    }

    Group {
        name: "apps512x512/mimetypes"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "512x512", "mimetypes", FileInfo.pathSeparator())
        files: ["application-x-valentina-i-measurements.png", "application-x-valentina-s-measurements.png"]
        fileTags: "freedesktop.512x512MimetypesIcons"
    }

    Group {
        condition: qbs.targetOS.contains("macos")
        fileTagsFilter: "qm"
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + FileInfo.pathSeparator() + "translations"
    }

    Group {
        name: "Diagrams"
        prefix: FileInfo.joinPaths(product.sourceDirectory, "share", "resources", FileInfo.pathSeparator())
        files: "diagrams.qrc"
        fileTags: "ebr.external_qrc"
    }

    Group {
        fileTagsFilter: "ebr.rcc"
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("unix") && !qbs.targetOS.contains("macos"))
                return FileInfo.joinPaths("share", "valentina", FileInfo.pathSeparator());
            else
                return buildconfig.installDataPath + FileInfo.pathSeparator();
        }
    }

    Group {
        name: "Tape RC"
        prefix: FileInfo.joinPaths(product.sourceDirectory, "share", "resources", FileInfo.pathSeparator())
        files: "tape.rc"
    }
}
