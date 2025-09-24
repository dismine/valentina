import qbs.FileInfo
import qbs.Utilities
import qbs.File

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
    Depends { name: "VGAnalyticsLib" }
    Depends { name: "Tape"; condition: qbs.targetOS.contains("macos") && buildconfig.enableMultiBundle }
    Depends { name: "Puzzle"; condition: qbs.targetOS.contains("macos") && buildconfig.enableMultiBundle }

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

    primaryApp: true
    name: "Valentina"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "Valentina" : "valentina"
    targetName: buildconfig.appTarget

    // On Windows .dll files are in bin folder
    Group {
        name: "xerces-c library (Windows)"
        condition: buildconfig.useConanPackages && buildconfig.conanXercesEnabled && qbs.targetOS.contains("windows")
        prefix: XercesC.binDirs[0] + "/"
        files: ["**/*" + cpp.dynamicLibrarySuffix]
        qbs.install: true
        qbs.installDir: buildconfig.installLibraryPath
        qbs.installSourceBase: XercesC.binDirs[0] + "/"
    }

    // On MacOS .so files are in lib folder
    Group {
        name: "xerces-c library (MacOS)"
        condition: buildconfig.useConanPackages && buildconfig.conanXercesEnabled && qbs.targetOS.contains("macos")
        prefix: XercesC.libraryPaths[0] + "/"
        files: ["**/*" + cpp.dynamicLibrarySuffix]
        qbs.install: true
        qbs.installDir: buildconfig.installLibraryPath
        qbs.installSourceBase: XercesC.libraryPaths[0] + "/"
    }

    Group {
        name: "Crashpad handler"
        condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled
        prefix: crashpad.binDirs[0] + "/"
        files: "crashpad_handler" + FileInfo.executableSuffix()
        qbs.install: true
        qbs.installDir: buildconfig.installBinaryPath
        qbs.installSourceBase: crashpad.binDirs[0] + "/"
    }

    Group {
        name: "ICU data"
        condition: buildconfig.useConanPackages &&
                   buildconfig.conanWithICUEnabled &&
                   (qbs.targetOS.contains("windows") || qbs.targetOS.contains("macos"))
        prefix: icudata.resources[0] + "/"
        files: ["**/*.dat"]
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + "/icu"
    }

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
            "vcmdexport.cpp",
            "vvalentinashortcutmanager.cpp",
            "vvalentinashortcutmanager.h",
        ]
    }

    Group {
        name: "Resources"
        prefix: "share/resources/"
        files: [
            "toolcursor.qrc", // Tools cursor icons
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

            defines.push('VALENTINA_PATH="' + path +'"');
            defines.push('TRANSLATIONS_DIR="' + exportingProduct.buildDirectory +'"');
            return defines;
        }
    }

    Group {
        name: "SVG Fonts"
        prefix: project.sourceDirectory + "/src/app/share/svgfonts/"
        files: [
            "**/*.svg"
        ]
        qbs.install: true
        qbs.installDir: product.buildconfig.installDataPath + "/svgfonts"
        qbs.installSourceBase: project.sourceDirectory + "/src/app/share/svgfonts/"
    }

    Properties {
        condition: qbs.targetOS.contains("macos")
        macdeployqt.targetApps: {
            var apps = [];
            if (!buildconfig.enableMultiBundle)
                apps.push("Tape", "Puzzle");

            if (buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled)
                apps.push("crashpad_handler");

            return apps;
        }
    }

    Group {
        condition: qbs.targetOS.contains("windows") && (qbs.architecture.contains("x86_64") || qbs.architecture.contains("x86"))
        name: "pdftops Windows"
        prefix: project.sourceDirectory + "/dist/win/"
        files: ["pdftops.exe"]
        qbs.install: true
        qbs.installDir: buildconfig.installBinaryPath
    }

    Group {
        name: "freedesktop"
        prefix: project.sourceDirectory + "/dist/"
        files: [
            "ua.com.smart-pattern." + product.targetName + ".metainfo.xml",
            "ua.com.smart-pattern." + product.targetName + ".desktop"
        ]
    }

    freedesktop2.desktopKeys: {
        var desktopKeys = {
            'Exec': FileInfo.joinPaths(qbs.installPrefix,
                                       product.installDir,
                                       product.targetName) + ' %F',
            'X-Application-Version': product.version,
        };

        if (buildconfig.enableAppImage) {
            var mimeTypes = [
                'application/x-valentina-pattern',
                'application/x-valentina-s-measurements',
                'application/x-valentina-i-measurements',
                'application/x-valentina-k-measurements',
                'application/x-valentina-layout'
            ];
            desktopKeys['MimeType'] = mimeTypes.join(';') + ';';
        }

        return (desktopKeys);
    }

    freedesktop2.hicolorRoot: project.sourceDirectory + "/share/icons/"

    Group {
        name: "hicolor"
        prefix: project.sourceDirectory + "/share/icons/"
        files: [
            "48x48/apps/valentina.png",
            "64x64/apps/valentina.png",
            "128x128/apps/valentina.png",
            "256x256/apps/valentina.png",
            "512x512/apps/valentina.png",
            "48x48/mimetypes/application-x-valentina-pattern.png",
            "64x64/mimetypes/application-x-valentina-pattern.png",
            "128x128/mimetypes/application-x-valentina-pattern.png",
            "256x256/mimetypes/application-x-valentina-pattern.png",
            "512x512/mimetypes/application-x-valentina-pattern.png",
        ]
        fileTags: "freedesktop.appIcon"
    }

    Group {
        name: "win deploy"
        condition: qbs.targetOS.contains("windows")
        prefix: project.sourceDirectory + "/"
        files: [
            "dist/win/valentina.ico",
            "dist/win/pattern.ico",
            "AUTHORS.txt",
            "LICENSE_GPL.txt",
            "README.md",
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
        prefix: project.sourceDirectory + "/dist/macos/valentina/"
        files: [
            "Info.plist",
            "valentina.xcassets"
        ]
    }

    Group {
        name: "ICNS"
        condition: qbs.targetOS.contains("macos")
        prefix: project.sourceDirectory + "/dist/macos/valentina-project.xcassets/"
        files: [
            "pattern.iconset",
            "layout.iconset",
            "i-measurements.iconset",
            "s-measurements.iconset",
            "k-measurements.iconset"
        ]
    }
}
