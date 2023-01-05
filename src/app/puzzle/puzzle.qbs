VApp {
    Depends { name: "buildconfig" }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "freedesktop" }
    Depends { name: "Qt"; submodules: ["widgets", "svg"] }
    Depends { name: "VMiscLib" }
    Depends { name: "VLayoutLib" }
    Depends { name: "IFCLib" }
    Depends { name: "VFormatLib" }
    Depends { name: "VWidgetsLib" }
    Depends { name: "FervorLib" }

    Properties {
        condition: cpp.defines.contains("APPIMAGE")
        cpp.dynamicLibraries: ["icudata", "icui18n", "icuuc"]
    }

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

    Group {
        name: "Precompiled headers"
        files: {
            var files = ["stable.h"];
            if (qbs.targetOS.contains("windows"))
                files.push("stable.cpp")
            return files;
        }
        fileTags: ["cpp_pch_src"]
    }

    cpp.includePaths: [product.sourceDirectory]
}
