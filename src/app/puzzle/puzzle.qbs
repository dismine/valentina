import qbs.FileInfo

VToolApp {
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

    cpp.includePaths: [product.sourceDirectory]

    Export {
        Depends { name: "cpp" }
        cpp.defines: {
            var defines = [];
            var extension = qbs.targetOS.contains("windows") ? ".exe" : "";
            defines.push('PUZZLE_BUILDDIR="' + exportingProduct.buildDirectory + FileInfo.pathSeparator() +
                         exportingProduct.targetName + extension +'"');
            return defines;
        }
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

    Rule {
        multiplex: true
        alwaysRun: true
        condition: qbs.targetOS.contains("windows") && (qbs.architecture.contains("x86_64") || qbs.architecture.contains("x86"))
        inputs: ["pdftops_dist_win"]
        outputFileTags: ["testSuit"]
        outputArtifacts: {
            var artifactNames = inputs["pdftops_dist_win"].map(function(file){
                return FileInfo.joinPaths(product.buildDirectory, file.fileName);
            });

            var artifacts = artifactNames.map(function(art){
                var a = {
                    filePath: art,
                    fileTags: ["testSuit"]
                }
                return a;
            });
            return artifacts;
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Preparing test suit";

            var sources = inputs["pdftops_dist_win"].map(function(artifact) {
                return artifact.filePath;
            });

            cmd.sources = sources;

            var destination = inputs["pdftops_dist_win"].map(function(artifact) {
                return FileInfo.joinPaths(product.buildDirectory, file.fileName);
            });
            cmd.destination = destination;
            cmd.sourceCode = function() {
                for (var i in sources) {
                    File.copy(sources[i], destination[i]);
                }
            };
            return [cmd];
        }
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

    Rule {
        multiplex: true
        alwaysRun: true
        condition: qbs.targetOS.contains("windows") && qbs.architecture.contains("x86_64") && qbs.buildVariant === "debug"
        inputs: ["pdftops_dist_macx"]
        outputFileTags: ["testSuit"]
        outputArtifacts: {
            var artifactNames = inputs["pdftops_dist_macx"].map(function(file){
                return FileInfo.joinPaths(product.buildDirectory, file.fileName);
            });

            var artifacts = artifactNames.map(function(art){
                var a = {
                    filePath: art,
                    fileTags: ["testSuit"]
                }
                return a;
            });
            return artifacts;
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Preparing test suit";

            var sources = inputs["pdftops_dist_macx"].map(function(artifact) {
                return artifact.filePath;
            });

            cmd.sources = sources;

            var destination = inputs["pdftops_dist_macx"].map(function(artifact) {
                return FileInfo.joinPaths(product.buildDirectory, file.fileName);
            });
            cmd.destination = destination;
            cmd.sourceCode = function() {
                for (var i in sources) {
                    File.copy(sources[i], destination[i]);
                }
            };
            return [cmd];
        }
    }
}
