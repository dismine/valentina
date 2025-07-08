import qbs.FileInfo
import qbs.File
import qbs.Utilities

VToolApp {
    Depends { name: "buildconfig" }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "Qt"; submodules: ["core", "widgets", "svg", "concurrent"] }
    Depends { name: "VMiscLib" }
    Depends { name: "VLayoutLib" }
    Depends { name: "IFCLib" }
    Depends { name: "VFormatLib" }
    Depends { name: "VWidgetsLib" }
    Depends { name: "FervorLib" }
    Depends { name: "multibundle"; }
    Depends { name: "VGAnalyticsLib" }

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

    name: "Puzzle"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "Puzzle" : "puzzle"
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

    files: [
        "main.cpp",
        "vpapplication.cpp",
        "vpcommandline.cpp",
        "vpcommands.cpp",
        "vpmainwindow.cpp",
        "vpsettings.cpp",
        "vptilefactory.cpp",
        "vpapplication.h",
        "vpcommandline.h",
        "vpcommands.h",
        "vpmainwindow.h",
        "vpsettings.h",
        "vptilefactory.h",
        "vpmainwindow.ui",
        "vpuzzleshortcutmanager.cpp",
        "vpuzzleshortcutmanager.h",
    ]

    Group {
        name: "dialogs"
        prefix: "dialogs/"
        files: [
            "configpages/puzzlepreferencesconfigurationpage.cpp",
            "configpages/puzzlepreferencespathpage.cpp",
            "configpages/puzzlepreferenceslayoutpage.cpp",
            "dialogpuzzlepreferences.cpp",
            "vpdialogabout.cpp",
            "dialogsavemanuallayout.cpp",
            "configpages/puzzlepreferencesconfigurationpage.h",
            "configpages/puzzlepreferencespathpage.h",
            "configpages/puzzlepreferenceslayoutpage.h",
            "dialogpuzzlepreferences.h",
            "vpdialogabout.h",
            "dialogsavemanuallayout.h",
            "configpages/puzzlepreferencesconfigurationpage.ui",
            "configpages/puzzlepreferencespathpage.ui",
            "configpages/puzzlepreferenceslayoutpage.ui",
            "dialogpuzzlepreferences.ui",
            "vpdialogabout.ui",
            "dialogsavemanuallayout.ui",
        ]
    }

    Group {
        name: "undocommands"
        prefix: "undocommands/"
        files: [
            "vpundoaddsheet.cpp",
            "vpundocommand.cpp",
            "vpundomovepieceonsheet.cpp",
            "vpundooriginmove.cpp",
            "vpundopiecemove.cpp",
            "vpundopiecerotate.cpp",
            "vpundopiecezvaluemove.cpp",
            "vpundoremovesheet.cpp",
            "vpundoaddsheet.h",
            "vpundocommand.h",
            "vpundomovepieceonsheet.h",
            "vpundooriginmove.h",
            "vpundopiecemove.h",
            "vpundopiecerotate.h",
            "vpundopiecezvaluemove.h",
            "vpundoremovesheet.h",
        ]
    }

    Group {
        name: "xml"
        prefix: "xml/"
        files: [
            "vplayoutfilereader.cpp",
            "vplayoutfilewriter.cpp",
            "vplayoutliterals.cpp",
            "vplayoutfilereader.h",
            "vplayoutfilewriter.h",
            "vplayoutliterals.h",
        ]
    }

    Group {
        name: "layout"
        prefix: "layout/"
        files: [
            "layoutdef.cpp",
            "vplayout.cpp",
            "vplayoutsettings.cpp",
            "vppiece.cpp",
            "vpsheet.cpp",
            "layoutdef.h",
            "vplayout.h",
            "vplayoutsettings.h",
            "vppiece.h",
            "vpsheet.h",
        ]
    }

    Group {
        name: "carousel"
        prefix: "carousel/"
        files: [
            "vpcarrousel.cpp",
            "vpcarrouselpiece.cpp",
            "vpcarrouselpiecelist.cpp",
            "vpmimedatapiece.cpp",
            "vpcarrousel.h",
            "vpcarrouselpiece.h",
            "vpcarrouselpiecelist.h",
            "vpmimedatapiece.h",
            "vpcarrousel.ui",
        ]
    }

    Group {
        name: "scene"
        prefix: "scene/"
        files: [
            "vpgraphicspiece.cpp",
            "vpgraphicspiececontrols.cpp",
            "vpgraphicssheet.cpp",
            "vpgraphicstilegrid.cpp",
            "vpmaingraphicsview.cpp",
            "scenedef.h",
            "vpgraphicspiece.h",
            "vpgraphicspiececontrols.h",
            "vpgraphicssheet.h",
            "vpgraphicstilegrid.h",
            "vpmaingraphicsview.h",
        ]
    }

    Group {
        name: "Resources"
        prefix: "share/resources/"
        files: [
            "puzzlecursor.qrc", // Tools cursor icons
            "puzzleicon.qrc",
        ]
    }

    cpp.includePaths: [product.sourceDirectory]

    Export {
        Depends { name: "cpp" }
        cpp.defines: {
            var defines = [];
            // TODO: If minimal qbs version is 1.23 replace with FileInfo.executableSuffix()
            var extension = qbs.targetOS.contains("windows") ? ".exe" : "";
            defines.push('PUZZLE_BUILDDIR="' + FileInfo.joinPaths(exportingProduct.buildDirectory, exportingProduct.targetName + extension) +'"');
            return defines;
        }
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

    Properties {
        condition: qbs.targetOS.contains("macos") && buildconfig.enableMultiBundle
        macdeployqt.targetApps: {
            var apps = [];

            if (buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled)
                apps.push("crashpad_handler");

            return apps;
        }
    }

    freedesktop2.hicolorRoot: project.sourceDirectory + "/share/icons/"

    Group {
        name: "hicolor"
        prefix: project.sourceDirectory + "/share/icons/"
        files: [
            "48x48/apps/puzzle.png",
            "64x64/apps/puzzle.png",
            "128x128/apps/puzzle.png",
            "256x256/apps/puzzle.png",
            "512x512/apps/puzzle.png",
            "48x48/mimetypes/application-x-valentina-layout.png",
            "64x64/mimetypes/application-x-valentina-layout.png",
            "128x128/mimetypes/application-x-valentina-layout.png",
            "256x256/mimetypes/application-x-valentina-layout.png",
            "512x512/mimetypes/application-x-valentina-layout.png"
        ]
        fileTags: "freedesktop.appIcon"
    }

    Group {
        name: "Puzzle RC"
        condition: qbs.targetOS.contains("windows")
        prefix: product.sourceDirectory + "/share/resources/"
        files: "puzzle.rc"
    }

    Group {
        name: "win deploy"
        condition: qbs.targetOS.contains("windows")
        prefix: project.sourceDirectory + "/dist/win/"
        files: [
            "layout.ico",
        ]
        qbs.install: true
        qbs.installDir: buildconfig.installAppPath
    }

    Group {
        name: "MacOS assets"
        condition: qbs.targetOS.contains("macos") && buildconfig.enableMultiBundle
        prefix: project.sourceDirectory + "/dist/macos/puzzle/"
        files: [
            "Info.plist",
            "puzzle.xcassets"
        ]
    }

    Group {
        name: "ICNS"
        condition: qbs.targetOS.contains("macos") && buildconfig.enableMultiBundle
        prefix: project.sourceDirectory + "/dist/macos/valentina-project.xcassets/"
        files: "layout.iconset"
    }

    Group {
        condition: product.qbs.targetOS.contains("macos") && product.buildconfig.enableMultiBundle
        name: "SVG Fonts"
        prefix: project.sourceDirectory + "/src/app/share/svgfonts/"
        files: [
            "**/*.svg"
        ]
        qbs.install: true
        qbs.installDir: product.buildconfig.installDataPath + "/svgfonts"
        qbs.installSourceBase: project.sourceDirectory + "/src/app/share/svgfonts/"
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
