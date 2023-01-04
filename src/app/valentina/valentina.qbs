VApp {
    Depends { name: "buildconfig" }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "freedesktop" }
    Depends { name: "Qt"; submodules: ["gui", "widgets", "xml", "svg", "printsupport", "xmlpatterns", "concurrent"] }
    Depends { name: "VPropertyExplorerLib" }
    Depends { name: "VPatternDBLib"; }
    Depends { name: "VWidgetsLib"; }
    Depends { name: "FervorLib"; }
    Depends { name: "IFCLib"; }
    Depends { name: "VLayoutLib"; }
    Depends { name: "VToolsLib"; }
    Depends { name: "VFormatLib"; }

    Depends {
        name: "Qt.winextras"
        condition: qbs.targetOS.contains("windows")
        versionAtLeast: "5.6"
        required: false
    }

    Properties {
        condition: cpp.defines.contains("APPIMAGE")
        cpp.dynamicLibraries: ["icudata", "icui18n", "icuuc"]
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

    Properties {
        // Breakpoints do not work if debug the app inside of bundle. In debug mode we turn off creating a bundle.
        // Probably it will breake some dependencies. Version for Mac designed to work inside an app bundle.
        condition: qbs.targetOS.contains("macos") && qbs.buildVariant == "debug"
        bundle.isBundle: false
    }

    Group {
        name: "valentina.png"
        condition: qbs.targetOS.contains("linux")
        files: [ "../../../share/icons/64x64/apps/valentina.png" ]
        qbs.install: true
        qbs.installDir: "share/pixmaps"
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
}
