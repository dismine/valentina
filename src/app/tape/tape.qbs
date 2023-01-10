import qbs.FileInfo
import qbs.File

VToolApp {
    Depends { name: "buildconfig" }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "Qt"; submodules: ["widgets", "svg"] }
    Depends { name: "VMiscLib"; }
    Depends { name: "VPatternDBLib"; }
    Depends { name: "FervorLib"; }
    Depends { name: "QMUParserLib"; }
    Depends { name: "VFormatLib"; }
    Depends { name: "VWidgetsLib"; }
    Depends { name: "VToolsLib"; }

    Properties {
        condition: cpp.defines.contains("APPIMAGE")
        cpp.dynamicLibraries: ["icudata", "icui18n", "icuuc"]
    }

    name: "Tape"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "Tape" : "tape"
    targetName: buildconfig.appTarget

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
            var extension = qbs.targetOS.contains("windows") ? ".exe" : "";
            defines.push('TAPE_BUILDDIR="' + exportingProduct.buildDirectory + FileInfo.pathSeparator() +
                         exportingProduct.targetName + extension +'"');
            return defines;
        }
    }

    Group {
        name: "Multisize tables"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "src", "app", "share", "tables", "multisize", FileInfo.pathSeparator())
        files: [
            "GOST_man_ru.vst"
        ]
        fileTags: ["multisize_tables"]
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + FileInfo.pathSeparator() + "tables" + FileInfo.pathSeparator() + "multisize"
    }

    Rule {
        multiplex: true
        alwaysRun: true
        inputs: ["multisize_tables"]
        outputFileTags: ["testSuit"]
        outputArtifacts: {
            var artifactNames = inputs["multisize_tables"].map(function(file){
                return FileInfo.joinPaths(product.buildDirectory, "tables", "multisize", file.fileName);
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
            cmd.description = "Preparing multisize tables";
            cmd.highlight = "filegen";

            var sources = inputs["multisize_tables"].map(function(artifact) {
                return artifact.filePath;
            });

            cmd.sources = sources;

            var destination = inputs["multisize_tables"].map(function(file) {
                return FileInfo.joinPaths(product.buildDirectory, "tables", "multisize", file.fileName);
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
        name: "Measurements templates"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "src", "app", "share", "tables", "templates", FileInfo.pathSeparator())
        files: [
            "template_all_measurements.vit",
            "t_Aldrich_Women.vit"
        ]
        fileTags: ["measurements_templates"]
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + FileInfo.pathSeparator() + "tables" + FileInfo.pathSeparator() + "templates"
    }
}
