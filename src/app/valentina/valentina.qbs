import qbs.FileInfo
import qbs.File

VToolApp {
    Depends { name: "buildconfig" }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "Qt"; submodules: ["widgets", "svg", "xmlpatterns", "concurrent"] }
    Depends { name: "VPatternDBLib"; }
    Depends { name: "VWidgetsLib"; }
    Depends { name: "FervorLib"; }
    Depends { name: "IFCLib"; }
    Depends { name: "VLayoutLib"; }
    Depends { name: "VToolsLib"; }
    Depends { name: "VFormatLib"; }
    Depends { name: "VMiscLib"; }

    Depends {
        name: "Qt.winextras"
        condition: qbs.targetOS.contains("windows")
        versionAtLeast: "5.6"
        required: false
    }

    Properties {
        condition: cpp.defines.contains("APPIMAGE")
        cpp.dynamicLibraries: ["icudata", "icui18n", "icuuc"]
    }

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

    Properties {
        // Breakpoints do not work if debug the app inside of bundle. In debug mode we turn off creating a bundle.
        // Probably it will breake some dependencies. Version for Mac designed to work inside an app bundle.
        condition: qbs.targetOS.contains("macos") && qbs.buildVariant == "debug"
        bundle.isBundle: false
    }

    Group {
        name: "valentina.png"
        condition: qbs.targetOS.contains("linux")
        files: [ "../../../share/icons/64x64/apps/valentina.png" ]
        qbs.install: true
        qbs.installDir: "share/pixmaps"
    }

    Export {
        Depends { name: "cpp" }
        cpp.defines: {
            var defines = [];
            var extension = qbs.targetOS.contains("windows") ? ".exe" : "";
            defines.push('VALENTINA_BUILDDIR="' + exportingProduct.buildDirectory + FileInfo.pathSeparator() +
                         exportingProduct.targetName + extension +'"');
            defines.push('TRANSLATIONS_DIR="' + exportingProduct.buildDirectory + FileInfo.pathSeparator() + 'translations"');
            return defines;
        }
    }

    Group {
        name: "Label templates"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "src", "app", "share", "labels", FileInfo.pathSeparator())
        files: [
            "def_pattern_label.xml",
            "def_piece_label.xml"
        ]
        fileTags: ["label_templates"]
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + FileInfo.pathSeparator() + "labels"
    }

    Rule {
        multiplex: true
        alwaysRun: true
        inputs: ["label_templates"]
        outputFileTags: ["testSuit"]
        outputArtifacts: {
            var artifactNames = inputs["label_templates"].map(function(file){
                return FileInfo.joinPaths(product.buildDirectory, "labels", file.fileName);
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
            cmd.description = "Preparing default labels";

            var sources = inputs["label_templates"].map(function(artifact) {
                return artifact.filePath;
            });

            cmd.sources = sources;

            var destination = inputs["label_templates"].map(function(file) {
                return FileInfo.joinPaths(product.buildDirectory, "labels", file.fileName);
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
