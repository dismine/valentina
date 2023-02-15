import qbs.File
import qbs.FileInfo
import qbs.TextFile
import qbs.Process

/**
  This module generates 'i18n.pro' artifact, which then acts as an input for 'lupdate' program, which in turn produces
  translation files, which are compiled by 'lrelease' program into 'qm' files, which can be loaded by an application.
  */
Module {
    Depends { name: "Qt.core" }

    additionalProductTypes: ["i18n"]

    /*
      Unfortunately you can not simply add empty files to the product, cause 'Qt.core' module has a rule, which calls 'lrelease' on
      every 'ts' file in the product and 'lrelease' triggers error if these files are empty. Additionaly 'lupdate' also triggers
      errors, when parsing 'pro' file. Instead this property can be used to create new translation file.
      */
    property stringList additionalTranslations: []

    // Explicitly trigger build even if build a product
    property bool update: false

    // Build with legacy way though .pro file
    property bool buildWithPro: true

    property string lupdateName: "lupdate"

    readonly property string qtTranslationsPath: qtTranslationsProbe.qtTranslationsPath

    Rule {
        condition: update && buildWithPro
        multiplex: true
        inputs: ["i18n.hpp", "i18n.src", "i18n.ui", "i18n.res", "i18n.ts"]

        prepare: {
            var proCmd = new JavaScriptCommand();
            proCmd.description = 'generating ' + output.filePath;
            proCmd.highlight = 'codegen';
            proCmd.sourceCode = function() {
                var f = new TextFile(output.filePath, TextFile.WriteOnly);
                try {
                    f.writeLine("lupdate_only {");
                    if (inputs["i18n.hpp"] !== undefined)
                        for (var i = 0; i < inputs["i18n.hpp"].length; i++)
                            f.writeLine("HEADERS += " + FileInfo.relativePath(product.sourceDirectory, inputs["i18n.hpp"][i].filePath));
                        f.writeLine("");
                    if (inputs["i18n.src"] !== undefined)
                        for (var i = 0; i < inputs["i18n.src"].length; i++)
                            f.writeLine("SOURCES += " + FileInfo.relativePath(product.sourceDirectory, inputs["i18n.src"][i].filePath));
                        f.writeLine("");
                    if (inputs["i18n.ui"] !== undefined)
                        for (var i = 0; i < inputs["i18n.ui"].length; i++)
                            f.writeLine("FORMS += " + FileInfo.relativePath(product.sourceDirectory, inputs["i18n.ui"][i].filePath));
                        f.writeLine("");
                    // lupdate processes QML files that are listed in the .qrc file
                    if (inputs["i18n.res"] !== undefined)
                        for (var i = 0; i < inputs["i18n.res"].length; i++)
                            f.writeLine("RESOURCES += " + FileInfo.relativePath(product.sourceDirectory, inputs["i18n.res"][i].filePath));
                    f.writeLine("}");

                    f.writeLine("");
                    if (inputs["i18n.ts"] !== undefined)
                        for (var i = 0; i < inputs["i18n.ts"].length; i++)
                            f.writeLine("TRANSLATIONS += " + FileInfo.relativePath(product.sourceDirectory, inputs["i18n.ts"][i].filePath));
                    for (var i = 0; i < product.i18n.additionalTranslations.length; i++) {
                        var targetDirectory = product.sourceDirectory + "/" + FileInfo.path(product.i18n.additionalTranslations[i]);
                        if (!File.exists(targetDirectory))
                            console.error("Directory '" + targetDirectory + "' does not exists. Please create it.");
                        f.writeLine("TRANSLATIONS += " + product.i18n.additionalTranslations[i]);
                    }
                } finally {
                    f.close();
                }
            }
            return [proCmd];
        }

        Artifact {
            filePath: product.sourceDirectory + "/" + product.name + ".i18n.pro"
            fileTags: ["i18n.pro"]
        }
    }

    Rule {
        condition: buildWithPro
        inputs: ["i18n.pro"]

        prepare: {
            var lupdateName = product.i18n.lupdateName;
            var cmdLupdate = new Command(product.Qt.core.binPath + '/' + lupdateName, ["-verbose", input.filePath]);
            cmdLupdate.description = "Invoking '" + lupdateName + "' program";
            cmdLupdate.highlight = 'filegen';

            var cmdClean = new JavaScriptCommand();
            cmdClean.description = "Removing " + input.fileName;
            cmdClean.highlight = "filegen";
            cmdClean.sourceCode = function() {
                File.remove(input.filePath);
            }
            return [cmdLupdate, cmdClean]
        }

        outputFileTags: ["i18n"]
    }

    Rule {
        condition: update && !buildWithPro
        multiplex: true
        inputs: ["i18n.hpp", "i18n.src", "i18n.ui", "i18n.ts"]

        prepare: {
            var proCmd = new JavaScriptCommand();
            proCmd.description = 'generating ' + output.filePath;
            proCmd.highlight = 'codegen';
            proCmd.sourceCode = function() {
                var f = new TextFile(output.filePath, TextFile.WriteOnly);
                try {
                    // Since Qt 5.13 lupdate supports passing a project description in JSON file. For producing such a
                    // description from .pro file we can use new tool lprodump. But tehnically we don't need it. We can
                    // totally fake format.
                    // JSON file structure:
                    // Project ::= {
                    //    string projectFile     // Name of the project file. (required)
                    //    string codec           // Source code codec. Valid values are
                    //                           // currently "utf-16" or "utf-8" (default).
                    //    string[] translations  // List of .ts files of the project. (required)
                    //    string[] includePaths  // List of include paths.
                    //    string[] sources       // List of source files. (required)
                    //    string[] excluded      // List of source files, which are
                    //                           // excluded for translation.
                    //    Project[] subProjects  // List of sub-projects.
                    // }
                    // It seems all we need are projectFile, sources and translations options.

                    var sources = [];
                    if (inputs["i18n.hpp"] !== undefined)
                        for (var i = 0; i < inputs["i18n.hpp"].length; i++)
                            sources.push(inputs["i18n.hpp"][i].filePath);

                    if (inputs["i18n.src"] !== undefined)
                        for (var i = 0; i < inputs["i18n.src"].length; i++)
                            sources.push(inputs["i18n.src"][i].filePath);

                    if (inputs["i18n.ui"] !== undefined)
                        for (var i = 0; i < inputs["i18n.ui"].length; i++)
                            sources.push(inputs["i18n.ui"][i].filePath);

                    // lupdate processes QML files that are listed in the .qrc file
                    if (inputs["i18n.res"] !== undefined)
                        for (var i = 0; i < inputs["i18n.res"].length; i++)
                            sources.push(inputs["i18n.res"][i].filePath);

                    var translations = [];
                    if (inputs["i18n.ts"] !== undefined)
                        for (var i = 0; i < inputs["i18n.ts"].length; i++)
                            translations.push(inputs["i18n.ts"][i].filePath);

                    for (var i = 0; i < product.i18n.additionalTranslations.length; i++) {
                        var targetDirectory = product.sourceDirectory + "/" + FileInfo.path(product.i18n.additionalTranslations[i]);
                        if (!File.exists(targetDirectory))
                            console.error("Directory '" + targetDirectory + "' does not exists. Please create it.");
                        translations.push(product.i18n.additionalTranslations[i]);
                    }

                    var project = {
                        projectFile: "", // Looks like can be empty
                        sources: sources.sort(),
                        translations: translations.sort()
                    };

                    f.write(JSON.stringify([project], null, 2));
                } finally {
                    f.close();
                }
            }
            return [proCmd];
        }

        Artifact {
            filePath: product.sourceDirectory + "/" + product.name + ".i18n.json"
            fileTags: ["i18n.json"]
        }
    }

    Rule {
        condition: !buildWithPro
        inputs: ["i18n.json"]

        prepare: {
            var lupdateName = product.i18n.lupdateName;
            var cmdLupdate = new Command(product.Qt.core.binPath + '/' + lupdateName, ["-verbose", "-project", input.filePath]);
            cmdLupdate.description = "Invoking '" + lupdateName + "' program";
            cmdLupdate.highlight = 'filegen';

            var cmdClean = new JavaScriptCommand();
            cmdClean.description = "Removing " + input.fileName;
            cmdClean.highlight = "filegen";
            cmdClean.sourceCode = function() {
                File.remove(input.filePath);
            }
            return [cmdLupdate, cmdClean]
        }

        outputFileTags: ["i18n"]
    }

    Probe {
        id: qtTranslationsProbe

        readonly property string binPath: product.Qt.core.binPath

        property string qtTranslationsPath

        configure: {
            var qmakeProcess = new Process();
            try {
                // TODO: If minimal qbs version is 1.23 replace with FileInfo.executableSuffix()
                var suffix = qbs.targetOS.contains("windows") ? ".exe" : "";
                var qmakePath = FileInfo.joinPaths(binPath, "qmake" + suffix);
                qmakeProcess.exec(qmakePath, ["-query"]);
                if (qmakeProcess.exitCode() !== 0) {
                    throw "The qmake executable '" + FileInfo.toNativeSeparators(qmakePath) + "' failed with exit code " +
                            qmakeProcess.exitCode() + ".";
                }
                while (!qmakeProcess.atEnd()) {
                    var line = qmakeProcess.readLine();
                    var index = (line || "").indexOf(":");
                    if (index !== -1) {
                        if (line.slice(0, index) === "QT_INSTALL_TRANSLATIONS") {
                            var path = line.slice(index + 1).trim();
                            if (path)
                                qtTranslationsPath = FileInfo.fromNativeSeparators(path);
                            break;
                        }
                    }
                }
            } finally {
                qmakeProcess.close();
            }
        }
    }
}
