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
    Depends { name: "VGAnalyticsLib" }

    Depends {
        name: "xerces-c";
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 &&
                   (!buildconfig.useConanPackages || (buildconfig.useConanPackages && !buildconfig.conanXercesEnabled))
    }

    Depends {
        name: "XercesC";
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 && buildconfig.useConanPackages &&
                   buildconfig.conanXercesEnabled
    }

    Depends {
        name: "icu";
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 &&
                   project.withTextCodec && project.withICUCodecs &&
                   buildconfig.useConanPackages && buildconfig.conanWithICUEnabled
    }

    Depends {
        name: "icudata"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 &&
                   project.withTextCodec && project.withICUCodecs &&
                   buildconfig.useConanPackages && buildconfig.conanWithICUEnabled
    }

    // Explicitly link to libcrypto and libssl to avoid error: Failed to load libssl/libcrypto.
    // Use moduleProviders.qbspkgconfig.extraPaths to define the missing dependency.
    // Explicit linking will help macdeployqt undertsand that we want to see them inside the bundle.
    Depends {
        name: "libcrypto"
        condition: qbs.targetOS.contains("macos") && Utilities.versionCompare(Qt.core.version, "6") >= 0
    }

    Depends {
        name: "libssl"
        condition: qbs.targetOS.contains("macos") && Utilities.versionCompare(Qt.core.version, "6") >= 0
    }

    name: "Tape"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "Tape" : "tape"
    targetName: buildconfig.appTarget
    multibundle.targetApps: ["Valentina"]

    Group {
        name: "xerces-c library (MacOS)"
        condition: buildconfig.useConanPackages && buildconfig.conanXercesEnabled && qbs.targetOS.contains("macos") && buildconfig.enableMultiBundle
        prefix: XercesC.libraryPaths[0] + "/"
        files: ["**/*" + cpp.dynamicLibrarySuffix]
        qbs.install: true
        qbs.installDir: buildconfig.installLibraryPath
        qbs.installSourceBase: XercesC.libraryPaths[0] + "/"
    }

    Group {
        name: "Crashpad handler"
        condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled && qbs.targetOS.contains("macos") && buildconfig.enableMultiBundle
        prefix: crashpad.binDirs[0] + "/"
        files: "crashpad_handler" + FileInfo.executableSuffix()
        qbs.install: true
        qbs.installDir: buildconfig.installBinaryPath
        qbs.installSourceBase: crashpad.binDirs[0] + "/"
    }

    Group {
        name: "ICU data (MacOS)"
        condition: buildconfig.useConanPackages && buildconfig.conanWithICUEnabled && qbs.targetOS.contains("macos") && buildconfig.enableMultiBundle
        prefix: icudata.resources[0] + "/"
        files: ["**/*.dat"]
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + "/icu"
    }

    Properties {
        condition: qbs.targetOS.contains("macos") && buildconfig.enableMultiBundle
        macdeployqt.targetApps: {
            var apps = [];

            if (buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled)
                apps.push("crashpad_handler");

            return apps;
        }
    }

    files: [
        "main.cpp",
        "tkmmainwindow.cpp",
        "tkmmainwindow.h",
        "tkmmainwindow.ui",
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
        "vtapeshortcutmanager.cpp",
        "vtapeshortcutmanager.h",
    ]

    Group {
        name: "dialogs"
        prefix: "dialogs/"
        files: [
            "configpages/tapepreferencespathpage.cpp",
            "configpages/tapepreferencespathpage.h",
            "configpages/tapepreferencespathpage.ui",
            "dialogdimensioncustomnames.cpp",
            "dialogdimensionlabels.cpp",
            "dialogknownmeasurementscsvcolumns.cpp",
            "dialogknownmeasurementscsvcolumns.h",
            "dialogknownmeasurementscsvcolumns.ui",
            "dialogmeasurementscsvcolumns.cpp",
            "dialognoknownmeasurements.cpp",
            "dialognoknownmeasurements.h",
            "dialognoknownmeasurements.ui",
            "dialogrestrictdimension.cpp",
            "dialogabouttape.cpp",
            "dialognewmeasurements.cpp",
            "dialogmdatabase.cpp",
            "dialogtapepreferences.cpp",
            "configpages/tapepreferencesconfigurationpage.cpp",
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

            if (qbs.targetOS.contains("macos")) {
                var appTarget = product.buildconfig.enableMultiBundle ? "Valentina" : exportingProduct.targetName;
                var installBinaryPath = FileInfo.joinPaths(product.buildconfig.installAppPath,
                                                           appTarget + ".app/Contents/MacOS")
                var path = FileInfo.joinPaths(product.qbs.installRoot + product.qbs.installPrefix,
                                              installBinaryPath,
                                              exportingProduct.targetName);
            } else {
                var path = FileInfo.joinPaths(exportingProduct.buildDirectory,
                                              exportingProduct.targetName + FileInfo.executableSuffix());
            }

            defines.push('TAPE_PATH="' + path +'"');
            return defines;
        }
    }

    freedesktop2.hicolorRoot: project.sourceDirectory + "/share/icons/"

    Group {
        name: "hicolor"
        prefix: project.sourceDirectory + "/share/icons/"
        files: [
            "48x48/apps/tape.png",
            "64x64/apps/tape.png",
            "128x128/apps/tape.png",
            "256x256/apps/tape.png",
            "512x512/apps/tape.png",
            "48x48/mimetypes/application-x-valentina-i-measurements.png",
            "48x48/mimetypes/application-x-valentina-s-measurements.png",
            "48x48/mimetypes/application-x-valentina-k-measurements.png",
            "64x64/mimetypes/application-x-valentina-i-measurements.png",
            "64x64/mimetypes/application-x-valentina-s-measurements.png",
            "64x64/mimetypes/application-x-valentina-k-measurements.png",
            "128x128/mimetypes/application-x-valentina-i-measurements.png",
            "128x128/mimetypes/application-x-valentina-s-measurements.png",
            "128x128/mimetypes/application-x-valentina-k-measurements.png",
            "256x256/mimetypes/application-x-valentina-i-measurements.png",
            "256x256/mimetypes/application-x-valentina-s-measurements.png",
            "256x256/mimetypes/application-x-valentina-k-measurements.png",
            "512x512/mimetypes/application-x-valentina-i-measurements.png",
            "512x512/mimetypes/application-x-valentina-s-measurements.png",
            "512x512/mimetypes/application-x-valentina-k-measurements.png",
        ]
        fileTags: "freedesktop.appIcon"
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
            "k-measurements.ico",
        ]
        qbs.install: true
        qbs.installDir: buildconfig.installAppPath
    }

    Group {
        name: "MacOS assets"
        condition: qbs.targetOS.contains("macos") && buildconfig.enableMultiBundle
        prefix: project.sourceDirectory + "/dist/macos/tape/"
        files: [
            "Info.plist",
            "tape.xcassets"
        ]
    }

    Group {
        name: "ICNS"
        condition: qbs.targetOS.contains("macos") && buildconfig.enableMultiBundle
        prefix: project.sourceDirectory + "/dist/macos/valentina-project.xcassets/"
        files: [
            "i-measurements.iconset",
            "s-measurements.iconset",
            "k-measurements.iconset"
        ]
    }

    Group {
        name: "freedesktop"
        condition: !buildconfig.enableAppImage && qbs.targetOS.contains("unix") && !qbs.targetOS.contains("macos")
        prefix: project.sourceDirectory + "/dist/"
        files: [
            "ua.com.smart-pattern." + product.targetName + ".desktop"
        ]
    }

    freedesktop2.desktopKeys: ({
       'Exec': FileInfo.joinPaths(qbs.installPrefix,
                                  product.installDir,
                                  product.targetName) + ' %F',
       'X-Application-Version': product.version,
    })
}
