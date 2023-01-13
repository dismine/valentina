import qbs.FileInfo

VToolApp {
    Depends { name: "buildconfig" }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "Qt"; submodules: ["widgets", "svg", "xmlpatterns", "concurrent"] }
    Depends { name: "VPatternDBLib"; }
    Depends { name: "VWidgetsLib"; }
    Depends { name: "FervorLib"; }
    Depends { name: "IFCLib"; }
    Depends { name: "VLayoutLib"; }
    Depends { name: "VToolsLib"; }
    Depends { name: "VFormatLib"; }
    Depends { name: "VMiscLib"; }

    Depends {
        name: "Qt.winextras"
        condition: qbs.targetOS.contains("windows")
        versionAtLeast: "5.6"
        required: false
    }

    name: "Valentina"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "Valentina" : "valentina"
    targetName: buildconfig.appTarget

    files: [
        "main.cpp",
        "mainwindow.cpp",
        "mainwindow.h",
        "mainwindowsnogui.cpp",
        "mainwindowsnogui.h",
        "version.h",
        "mainwindow.ui"
    ]

    Group {
        name: "dialogs"
        prefix: "dialogs/"
        files: [
            "dialogaddbackgroundimage.h",
            "dialogs.h",
            "dialogincrements.h",
            "dialoghistory.h",
            "dialogpatternproperties.h",
            "dialognewpattern.h",
            "dialogaboutapp.h",
            "dialoglayoutsettings.h",
            "dialoglayoutprogress.h",
            "dialogsavelayout.h",
            "vwidgetbackgroundimages.h",
            "vwidgetgroups.h",
            "vwidgetdetails.h",
            "dialogpreferences.h",
            "configpages/preferencesconfigurationpage.h",
            "configpages/preferencespatternpage.h",
            "configpages/preferencespathpage.h",
            "dialogdatetimeformats.h",
            "dialogknownmaterials.h",
            "dialogfinalmeasurements.h",

            "dialogaddbackgroundimage.cpp",
            "dialogincrements.cpp",
            "dialoghistory.cpp",
            "dialogpatternproperties.cpp",
            "dialognewpattern.cpp",
            "dialogaboutapp.cpp",
            "dialoglayoutsettings.cpp",
            "dialoglayoutprogress.cpp",
            "dialogsavelayout.cpp",
            "vwidgetbackgroundimages.cpp",
            "vwidgetgroups.cpp",
            "vwidgetdetails.cpp",
            "dialogpreferences.cpp",
            "configpages/preferencesconfigurationpage.cpp",
            "configpages/preferencespatternpage.cpp",
            "configpages/preferencespathpage.cpp",
            "dialogdatetimeformats.cpp",
            "dialogknownmaterials.cpp",
            "dialogfinalmeasurements.cpp",

            "dialogaddbackgroundimage.ui",
            "dialogincrements.ui",
            "dialoghistory.ui",
            "dialogpatternproperties.ui",
            "dialognewpattern.ui",
            "dialogaboutapp.ui",
            "dialoglayoutsettings.ui",
            "dialoglayoutprogress.ui",
            "dialogsavelayout.ui",
            "vwidgetbackgroundimages.ui",
            "vwidgetgroups.ui",
            "vwidgetdetails.ui",
            "dialogpreferences.ui",
            "configpages/preferencesconfigurationpage.ui",
            "configpages/preferencespatternpage.ui",
            "configpages/preferencespathpage.ui",
            "dialogdatetimeformats.ui",
            "dialogknownmaterials.ui",
            "dialogfinalmeasurements.ui"
        ]
    }

    Group {
        name: "xml"
        prefix: "xml/"
        files: [
            "vpattern.h",
            "vpattern.cpp"
        ]
    }

    Group {
        name: "core"
        prefix: "core/"
        files: [
            "vapplication.h",
            "vformulaproperty.h",
            "vformulapropertyeditor.h",
            "vtooloptionspropertybrowser.h",
            "vcmdexport.h",

            "vapplication.cpp",
            "vformulaproperty.cpp",
            "vformulapropertyeditor.cpp",
            "vtooloptionspropertybrowser.cpp",
            "vcmdexport.cpp"
        ]
    }

    Group {
        name: "Resources"
        prefix: "share/resources/"
        files: [
            "cursor.qrc", // Tools cursor icons
            "toolicon.qrc",
        ]
    }

    Properties {
        condition: qbs.targetOS.contains("macos")
        ib.appIconName: "Valentina"
    }

    Export {
        Depends { name: "cpp" }
        cpp.defines: {
            var defines = [];
            var extension = qbs.targetOS.contains("windows") ? ".exe" : "";
            defines.push('VALENTINA_BUILDDIR="' + exportingProduct.buildDirectory + FileInfo.pathSeparator() +
                         exportingProduct.targetName + extension +'"');
            defines.push('TRANSLATIONS_DIR="' + exportingProduct.buildDirectory + FileInfo.pathSeparator() + 'translations"');
            return defines;
        }
    }

    Group {
        name: "Label templates"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "src", "app", "share", "labels", FileInfo.pathSeparator())
        files: [
            "def_pattern_label.xml",
            "def_piece_label.xml"
        ]
        fileTags: ["label_templates"]
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + FileInfo.pathSeparator() + "labels"
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
        condition: qbs.targetOS.contains("windows") && (qbs.architecture.contains("x86_64") || qbs.architecture.contains("x86"))
        name: "pdftops Windows"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "dist", "win", FileInfo.pathSeparator())
        files: ["pdftops.exe"]
        fileTags: ["pdftops_dist_win"]
        qbs.install: true
        qbs.installDir: buildconfig.installBinaryPath
    }

    Group {
        condition: qbs.targetOS.contains("macos") && qbs.architecture.contains("x86_64")
        name: "pdftops MacOS"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "dist", "macx", "bin64", FileInfo.pathSeparator())
        files: ["pdftops"]
        fileTags: ["pdftops_dist_macx"]
        qbs.install: true
        qbs.installDir: buildconfig.installBinaryPath
    }

    Group {
        name: "48x48/apps"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "48x48", "apps", FileInfo.pathSeparator())
        files: "valentina.png"
        fileTags: "freedesktop.48x48AppsIcons"
    }

    Group {
        name: "64x64/apps"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "64x64", "apps", FileInfo.pathSeparator())
        files: "valentina.png"
        fileTags: "freedesktop.64x64AppsIcons"
    }

    Group {
        name: "128x128/apps"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "128x128", "apps", FileInfo.pathSeparator())
        files: "valentina.png"
        fileTags: "freedesktop.128x128AppsIcons"
    }

    Group {
        name: "256x256/apps"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "256x256", "apps", FileInfo.pathSeparator())
        files: "valentina.png"
        fileTags: "freedesktop.256x256AppsIcons"
    }

    Group {
        name: "512x512/apps"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "512x512", "apps", FileInfo.pathSeparator())
        files: "valentina.png"
        fileTags: "freedesktop.512x512AppsIcons"
    }

    Group {
        name: "apps48x48/mimetypes"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "48x48", "mimetypes", FileInfo.pathSeparator())
        files: "application-x-valentina-pattern.png"
        fileTags: "freedesktop.48x48MimetypesIcons"
    }

    Group {
        name: "apps64x64/mimetypes"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "64x64", "mimetypes", FileInfo.pathSeparator())
        files: "application-x-valentina-pattern.png"
        fileTags: "freedesktop.64x64MimetypesIcons"
    }

    Group {
        name: "apps128x128/mimetypes"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "128x128", "mimetypes", FileInfo.pathSeparator())
        files: "application-x-valentina-pattern.png"
        fileTags: "freedesktop.128x128MimetypesIcons"
    }

    Group {
        name: "apps256x256/mimetypes"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "256x256", "mimetypes", FileInfo.pathSeparator())
        files: "application-x-valentina-pattern.png"
        fileTags: "freedesktop.256x256MimetypesIcons"
    }

    Group {
        name: "apps512x512/mimetypes"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "share", "icons", "512x512", "mimetypes", FileInfo.pathSeparator())
        files: "application-x-valentina-pattern.png"
        fileTags: "freedesktop.512x512MimetypesIcons"
    }

    Group {
        fileTagsFilter: "qm"
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + FileInfo.pathSeparator() + "translations"
    }
}
