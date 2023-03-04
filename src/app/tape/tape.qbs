import qbs.FileInfo
import qbs.Utilities

VToolApp {
    Depends { name: "buildconfig" }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "Qt"; submodules: ["core", "widgets", "svg"] }
    Depends { name: "VMiscLib"; }
    Depends { name: "VPatternDBLib"; }
    Depends { name: "FervorLib"; }
    Depends { name: "QMUParserLib"; }
    Depends { name: "VFormatLib"; }
    Depends { name: "VWidgetsLib"; }
    Depends { name: "VToolsLib"; }
    Depends { name: "ebr" }
    Depends { name: "multibundle"; }
    Depends { name: "conan.XercesC"; condition: buildconfig.useConanPackages }

    name: "Tape"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "Tape" : "tape"
    targetName: buildconfig.appTarget
    multibundle.targetApps: ["Valentina"]

    Properties {
        condition: buildconfig.useConanPackages && qbs.targetOS.contains("macos") && buildconfig.enableMultiBundle
        conan.XercesC.libInstallDir: qbs.installPrefix + "/" + buildconfig.installLibraryPath
        conan.XercesC.installLib: true
    }

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
            // TODO: If minimal qbs version is 1.23 replace with FileInfo.executableSuffix(
            var extension = qbs.targetOS.contains("windows") ? ".exe" : "";
            defines.push('TAPE_BUILDDIR="' + FileInfo.joinPaths(exportingProduct.buildDirectory, exportingProduct.targetName + extension) +'"');
            return defines;
        }
    }

    Group {
        name: "Multisize tables"
        prefix: project.sourceDirectory + "/src/app/share/tables/multisize/"
        files: [
            "GOST_man_ru.vst"
        ]
        fileTags: ["multisize_tables"]
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("macos") && !buildconfig.enableMultiBundle)
                return buildconfig.installAppPath + "/Valentina.app/Contents/Resources/tables/multisize"

            return buildconfig.installDataPath + "/tables/multisize"
        }
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
        qbs.installDir: {
            if (qbs.targetOS.contains("macos") && !buildconfig.enableMultiBundle)
                return buildconfig.installAppPath + "/Valentina.app/Contents/Resources/tables/templates"

            return buildconfig.installDataPath + "/tables/templates"
        }
    }

    Group {
        name: "48x48/apps"
        prefix: project.sourceDirectory + "/share/icons/48x48/apps/"
        files: "tape.png"
        fileTags: "freedesktop.48x48AppsIcons"
    }

    Group {
        name: "64x64/apps"
        prefix: project.sourceDirectory + "/share/icons/64x64/apps/"
        files: "tape.png"
        fileTags: "freedesktop.64x64AppsIcons"
    }

    Group {
        name: "128x128/apps"
        prefix: project.sourceDirectory + "/share/icons/128x128/apps/"
        files: "tape.png"
        fileTags: "freedesktop.128x128AppsIcons"
    }

    Group {
        name: "256x256/apps"
        prefix: project.sourceDirectory + "/share/icons/256x256/apps/"
        files: "tape.png"
        fileTags: "freedesktop.256x256AppsIcons"
    }

    Group {
        name: "512x512/apps"
        prefix: project.sourceDirectory + "/share/icons/512x512/apps/"
        files: "tape.png"
        fileTags: "freedesktop.512x512AppsIcons"
    }

    Group {
        name: "apps48x48/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/48x48/mimetypes/"
        files: ["application-x-valentina-i-measurements.png", "application-x-valentina-s-measurements.png"]
        fileTags: "freedesktop.48x48MimetypesIcons"
    }

    Group {
        name: "apps64x64/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/64x64/mimetypes/"
        files: ["application-x-valentina-i-measurements.png", "application-x-valentina-s-measurements.png"]
        fileTags: "freedesktop.64x64MimetypesIcons"
    }

    Group {
        name: "apps128x128/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/128x128/mimetypes/"
        files: ["application-x-valentina-i-measurements.png", "application-x-valentina-s-measurements.png"]
        fileTags: "freedesktop.128x128MimetypesIcons"
    }

    Group {
        name: "apps256x256/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/256x256/mimetypes/"
        files: ["application-x-valentina-i-measurements.png", "application-x-valentina-s-measurements.png"]
        fileTags: "freedesktop.256x256MimetypesIcons"
    }

    Group {
        name: "apps512x512/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/512x512/mimetypes/"
        files: ["application-x-valentina-i-measurements.png", "application-x-valentina-s-measurements.png"]
        fileTags: "freedesktop.512x512MimetypesIcons"
    }

    Group {
        name: "Diagrams"
        prefix: product.sourceDirectory + "/share/resources/"
        files: "diagrams.qrc"
        fileTags: "ebr.external_qrc"
    }

    Group {
        fileTagsFilter: "ebr.rcc"
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("unix") && !qbs.targetOS.contains("macos"))
                return "share/valentina/";

            if (qbs.targetOS.contains("macos") && !buildconfig.enableMultiBundle)
                return buildconfig.installAppPath + "/Valentina.app/Contents/Resources"

            return buildconfig.installDataPath + "/";
        }
    }

    Group {
        name: "Tape RC"
        condition: qbs.targetOS.contains("windows")
        prefix: product.sourceDirectory + "/share/resources/"
        files: "tape.rc"
    }

    Group {
        name: "win deploy"
        condition: qbs.targetOS.contains("windows")
        prefix: project.sourceDirectory + "/dist/win/"
        files: [
            "i-measurements.ico",
            "s-measurements.ico",
        ]
        qbs.install: true
        qbs.installDir: buildconfig.installAppPath
    }

    Group {
        name: "MacOS assets"
        condition: qbs.targetOS.contains("macos") && buildconfig.enableMultiBundle
        prefix: project.sourceDirectory + "/dist/macx/tape/"
        files: [
            "Info.plist",
            "tape.xcassets"
        ]
    }

    Group {
        name: "ICNS"
        condition: qbs.targetOS.contains("macos") && buildconfig.enableMultiBundle
        prefix: project.sourceDirectory + "/dist/macx/valentina-project.xcassets/"
        files: [
            "i-measurements.iconset",
            "s-measurements.iconset"
        ]
    }
}
