import qbs.FileInfo
import qbs.Utilities

VToolApp {
    Depends { name: "buildconfig" }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "Qt"; submodules: ["core", "widgets", "svg", "concurrent"] }
    Depends { name: "VPatternDBLib"; }
    Depends { name: "VWidgetsLib"; }
    Depends { name: "FervorLib"; }
    Depends { name: "IFCLib"; }
    Depends { name: "VLayoutLib"; }
    Depends { name: "VToolsLib"; }
    Depends { name: "VFormatLib"; }
    Depends { name: "VMiscLib"; }

    Depends {
        name: "Qt.xmlpatterns"
        condition: Utilities.versionCompare(Qt.core.version, "6") < 0
    }

    Depends {
        name: "Qt.winextras"
        condition: qbs.targetOS.contains("windows")
        versionAtLeast: "5.6"
        required: false
    }

    primaryApp: true

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
            defines.push('VALENTINA_BUILDDIR="' + FileInfo.joinPaths(exportingProduct.buildDirectory, exportingProduct.targetName + extension) +'"');
            defines.push('TRANSLATIONS_DIR="' + FileInfo.joinPaths(exportingProduct.buildDirectory, 'translations') +'"');
            return defines;
        }
    }

    Group {
        name: "Label templates"
        prefix: project.sourceDirectory + "/src/app/share/labels/"
        files: [
            "def_pattern_label.xml",
            "def_piece_label.xml"
        ]
        fileTags: ["label_templates"]
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + "/labels"
    }

    Group {
        name: "Multisize tables"
        prefix: project.sourceDirectory + "/src/app/share/tables/multisize/"
        files: [
            "GOST_man_ru.vst"
        ]
        fileTags: ["multisize_tables"]
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + "/tables/multisize"
    }

    Group {
        name: "Measurements templates"
        prefix: project.sourceDirectory + "/src/app/share/tables/templates/"
        files: [
            "template_all_measurements.vit",
            "t_Aldrich_Women.vit"
        ]
        fileTags: ["measurements_templates"]
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + "/tables/templates"
    }

    Group {
        condition: qbs.targetOS.contains("windows") && (qbs.architecture.contains("x86_64") || qbs.architecture.contains("x86"))
        name: "pdftops Windows"
        prefix: project.sourceDirectory + "/dist/win/"
        files: ["pdftops.exe"]
        fileTags: ["pdftops_dist_win"]
        qbs.install: true
        qbs.installDir: buildconfig.installBinaryPath
    }

    Group {
        condition: qbs.targetOS.contains("macos") && qbs.architecture.contains("x86_64")
        name: "pdftops MacOS"
        prefix: project.sourceDirectory + "/dist/macx/bin64/"
        files: ["pdftops"]
        fileTags: ["pdftops_dist_macx"]
        qbs.install: true
        qbs.installDir: buildconfig.installBinaryPath
    }

    Group {
        name: "freedesktop"
        prefix: project.sourceDirectory + "/dist/"
        files: [
            "ua.com.smart-pattern." + product.targetName + ".metainfo.xml"
        ]
    }

    Group {
        name: "48x48/apps"
        prefix: project.sourceDirectory + "/share/icons/48x48/apps/"
        files: "valentina.png"
        fileTags: "freedesktop.48x48AppsIcons"
    }

    Group {
        name: "64x64/apps"
        prefix: project.sourceDirectory + "/share/icons/64x64/apps/"
        files: "valentina.png"
        fileTags: "freedesktop.64x64AppsIcons"
    }

    Group {
        name: "128x128/apps"
        prefix: project.sourceDirectory + "/share/icons/128x128/apps/"
        files: "valentina.png"
        fileTags: "freedesktop.128x128AppsIcons"
    }

    Group {
        name: "256x256/apps"
        prefix: project.sourceDirectory + "/share/icons/256x256/apps/"
        files: "valentina.png"
        fileTags: "freedesktop.256x256AppsIcons"
    }

    Group {
        name: "512x512/apps"
        prefix: project.sourceDirectory + "/share/icons/512x512/apps/"
        files: "valentina.png"
        fileTags: "freedesktop.512x512AppsIcons"
    }

    Group {
        name: "apps48x48/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/48x48/mimetypes/"
        files: "application-x-valentina-pattern.png"
        fileTags: "freedesktop.48x48MimetypesIcons"
    }

    Group {
        name: "apps64x64/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/64x64/mimetypes/"
        files: "application-x-valentina-pattern.png"
        fileTags: "freedesktop.64x64MimetypesIcons"
    }

    Group {
        name: "apps128x128/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/128x128/mimetypes/"
        files: "application-x-valentina-pattern.png"
        fileTags: "freedesktop.128x128MimetypesIcons"
    }

    Group {
        name: "apps256x256/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/256x256/mimetypes/"
        files: "application-x-valentina-pattern.png"
        fileTags: "freedesktop.256x256MimetypesIcons"
    }

    Group {
        name: "apps512x512/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/512x512/mimetypes/"
        files: "application-x-valentina-pattern.png"
        fileTags: "freedesktop.512x512MimetypesIcons"
    }

    Group {
        name: "win deploy"
        condition: qbs.targetOS.contains("windows")
        prefix: project.sourceDirectory + "/"
        files: [
            "dist/win/valentina.ico",
            "dist/win/pattern.ico",
            "dist/win/EUDC.TTE",
            "AUTHORS.txt",
            "LICENSE_GPL.txt",
            "README.txt",
            "ChangeLog.txt",
            "share/qtlogging.ini"
        ]
        qbs.install: true
        qbs.installDir: buildconfig.installAppPath
    }

    Group {
        name: "Valentina RC"
        condition: qbs.targetOS.contains("windows")
        prefix: product.sourceDirectory + "/share/resources/"
        files: "valentina.rc"
    }

    Group {
        name: "MacOS assets"
        condition: qbs.targetOS.contains("macos")
        prefix: project.sourceDirectory + "/dist/macx/valentina/"
        files: [
            "Info.plist",
            "valentina.xcassets"
        ]
    }

    Group {
        name: "ICNS"
        condition: qbs.targetOS.contains("macos")
        prefix: project.sourceDirectory + "/dist/macx/valentina-project.xcassets/"
        files: [
            "pattern.iconset",
            "layout.iconset",
            "i-measurements.iconset",
            "s-measurements.iconset"
        ]
    }
}