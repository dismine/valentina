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
    Depends { name: "pdftops"; condition: qbs.targetOS.contains("macos") }
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

    primaryApp: true
    name: "Valentina"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "Valentina" : "valentina"
    targetName: buildconfig.appTarget
    type: base.concat("install_root_svg_fonts")

    Properties {
        condition: buildconfig.useConanPackages && (qbs.targetOS.contains("windows") || qbs.targetOS.contains("macos"))
        conan.XercesC.libInstallDir: qbs.installPrefix + "/" + buildconfig.installLibraryPath
        conan.XercesC.binInstallDir: qbs.installPrefix + "/" + buildconfig.installBinaryPath
        conan.XercesC.installLib: {
            if (qbs.targetOS.contains("windows"))
                return false
            return true
        }
        conan.XercesC.installBin: {
            if (qbs.targetOS.contains("windows"))
                return true
            return false
        }
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
            "vcmdexport.cpp"
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
            // TODO: If minimal qbs version is 1.23 replace with FileInfo.executableSuffix(
            var extension = qbs.targetOS.contains("windows") ? ".exe" : "";
            defines.push('VALENTINA_BUILDDIR="' + FileInfo.joinPaths(exportingProduct.buildDirectory, exportingProduct.targetName + extension) +'"');
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
        fileTags:["svg_fonts"]
    }

    Rule {
        inputs: ["svg_fonts"]
        Artifact {
            filePath: {
                var dstDir = product.qbs.installRoot + product.qbs.installPrefix + "/" +
                        product.buildconfig.installDataPath + "/svgfonts";
                return dstDir + "/" + input.filePath.split("src/app/share/svgfonts/")[1];
            }
            fileTags: ["install_root_svg_fonts"]
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Installing " + input.fileName;
            cmd.highlight = "codegen";
            cmd.sourceCode = function() {
                File.copy(input.filePath, output.filePath);
            }
            return [cmd];
        }
    }

    Properties {
        condition: qbs.targetOS.contains("macos")
        macdeployqt.targetApps: {
            var apps = [];
            if (!buildconfig.enableMultiBundle)
                apps.push("Tape", "Puzzle");

            if (pdftops.pdftopsPresent)
                apps.push("pdftops");

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
        name: "pdftops MacOS"
        condition: qbs.targetOS.contains("macos") && pdftops.pdftopsPresent
        files: [pdftops.pdftopsPath]
        fileTags: ["pdftops.in"]
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
