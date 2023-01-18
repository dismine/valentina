import qbs.FileInfo

VToolApp {
    Depends { name: "buildconfig" }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "Qt"; submodules: ["widgets", "svg"] }
    Depends { name: "VMiscLib" }
    Depends { name: "VLayoutLib" }
    Depends { name: "IFCLib" }
    Depends { name: "VFormatLib" }
    Depends { name: "VWidgetsLib" }
    Depends { name: "FervorLib" }

    name: "Puzzle"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "Puzzle" : "puzzle"
    targetName: buildconfig.appTarget

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
            "cursor.qrc", // Tools cursor icons
            "puzzleicon.qrc",
        ]
    }

    cpp.includePaths: [product.sourceDirectory]

    Export {
        Depends { name: "cpp" }
        cpp.defines: {
            var defines = [];
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
        name: "48x48/apps"
        prefix: project.sourceDirectory + "/share/icons/48x48/apps/"
        files: "puzzle.png"
        fileTags: "freedesktop.48x48AppsIcons"
    }

    Group {
        name: "64x64/apps"
        prefix: project.sourceDirectory + "/share/icons/64x64/apps/"
        files: "puzzle.png"
        fileTags: "freedesktop.64x64AppsIcons"
    }

    Group {
        name: "128x128/apps"
        prefix: project.sourceDirectory + "/share/icons/128x128/apps/"
        files: "puzzle.png"
        fileTags: "freedesktop.128x128AppsIcons"
    }

    Group {
        name: "256x256/apps"
        prefix: project.sourceDirectory + "/share/icons/256x256/apps/"
        files: "puzzle.png"
        fileTags: "freedesktop.256x256AppsIcons"
    }

    Group {
        name: "512x512/apps"
        prefix: project.sourceDirectory + "/share/icons/512x512/apps/"
        files: "puzzle.png"
        fileTags: "freedesktop.512x512AppsIcons"
    }

    Group {
        name: "apps48x48/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/48x48/mimetypes/"
        files: "application-x-valentina-layout.png"
        fileTags: "freedesktop.48x48MimetypesIcons"
    }

    Group {
        name: "apps64x64/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/64x64/mimetypes/"
        files: "application-x-valentina-layout.png"
        fileTags: "freedesktop.64x64MimetypesIcons"
    }

    Group {
        name: "apps128x128/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/128x128/mimetypes/"
        files: "application-x-valentina-layout.png"
        fileTags: "freedesktop.128x128MimetypesIcons"
    }

    Group {
        name: "apps256x256/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/256x256/mimetypes/"
        files: "application-x-valentina-layout.png"
        fileTags: "freedesktop.256x256MimetypesIcons"
    }

    Group {
        name: "apps512x512/mimetypes"
        prefix: project.sourceDirectory + "/share/icons/512x512/mimetypes/"
        files: "application-x-valentina-layout.png"
        fileTags: "freedesktop.512x512MimetypesIcons"
    }

    Group {
        condition: qbs.targetOS.contains("macos")
        fileTagsFilter: "qm"
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + "/translations"
    }

    Group {
        name: "Puzzle RC"
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
}
