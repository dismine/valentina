import qbs.FileInfo
import qbs.File

VApp {
    Depends { name: "freedesktop2" }
    Depends { name: "tenv" }
    Depends { name: "windeployqt"; }
    Depends { name: "i18nconfig"; }
    Depends { name: "i18n"; }

    version: "0.7.52"
    install: true
    installDir: buildconfig.installAppPath
    installDebugInformation: true
    consoleApplication: false
    bundle.isBundle: qbs.buildVariant === "release"

    Properties {
        // Breakpoints do not work if debug the app inside of bundle. In debug mode we turn off creating a bundle.
        // Probably it will breake some dependencies. Version for Mac designed to work inside an app bundle.
        condition: qbs.targetOS.contains("macos") && qbs.buildVariant == "debug"
        bundle.isBundle: false
    }

    Properties {
        condition: buildconfig.enableAppImage && qbs.targetOS.contains("unix") && !qbs.targetOS.contains("macos")
        cpp.dynamicLibraries: ["icudata", "icui18n", "icuuc"]
    }

    Group {
        name: "freedesktop"
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

    Group {
        name: "Translations"
        prefix: project.sourceDirectory + "/share/translations/"
        files: {
            var files = [];

            var locales = i18nconfig.translationLocales;

            for (var i = 0; i < locales.length; i++) {
                files.push("valentina_" + locales[i] + ".ts");
            }

            var pmSystems = i18nconfig.pmSystems;

            for (var i = 0; i < pmSystems.length; i++) {
                for (var j = 0; j < locales.length; j++) {
                    files.push("measurements_" + pmSystems[i] + "_" + locales[j] + ".ts");
                }
            }

            return files;
        }
    }

    Group {
        condition: !qbs.targetOS.contains("macos") || (qbs.targetOS.contains("macos") && !bundle.isBundle)
        fileTagsFilter: "qm"
        qbs.install: true
        qbs.installDir: buildconfig.installDataPath + "/translations"
    }

    Rule {
        multiplex: true
        condition: qbs.targetOS.contains("macos") && bundle.isBundle
        inputs: ["qm"]
        outputFileTags: ["bundle.qm", "bundle.content"]
        outputArtifacts: {
            var locales = product.i18nconfig.translationLocales;
            var artifactNames = [];

            for (var i = 0; i < locales.length; i++) {
                const lprojDir = FileInfo.joinPaths(product.buildDirectory, product.bundle.bundleName,
                                                    "Contents", "Resources", "translations", locales[i] + product.bundle.localizedResourcesFolderSuffix);

                var qmRex = new RegExp('.*_' + locales[i] + '\.qm$', 'g');
                artifactNames = artifactNames.concat((inputs["qm"] || []).filter(function(file){
                    return qmRex.exec(file.fileName);
                }).map(function(file){
                    return FileInfo.joinPaths(lprojDir, file.fileName);
                }));

                artifactNames.push(FileInfo.joinPaths(lprojDir, "Localizable.strings"));

                const qtTranslationsMask = [
                    "qt_",
                    "qtbase_",
                    "qtxmlpatterns_"
                ];

                qtTranslationsMask.forEach(function(mask) {
                    var qmFile = FileInfo.joinPaths(product.i18n.qtTranslationsPath, mask + locales[i] + ".qm");
                    if (File.exists(qmFile)) {
                        artifactNames.push(FileInfo.joinPaths(lprojDir, mask + locales[i] + ".qm"));
                    } else {
                        const lang = locales[i].split('_')[0];
                        qmFile = FileInfo.joinPaths(product.i18n.qtTranslationsPath, mask + lang + ".qm");
                        if (File.exists(qmFile))
                            artifactNames.push(FileInfo.joinPaths(lprojDir, mask + lang + ".qm"));
                    }
                });
            }

            var artifacts = artifactNames.map(function(art){
                var a = {
                    filePath: art,
                    fileTags: ["bundle.qm", "bundle.content"]
                }
                return a;
            });
            return artifacts;
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Preparing Valentina translations";
            cmd.highlight = "filegen";

            var data = [];
            const locales = product.i18nconfig.translationLocales;

            for (var i = 0; i < locales.length; i++) {
                const qmRex = new RegExp('.*_' + locales[i] + '.qm$', 'g');
                const src = (inputs["qm"] || []).filter(function(file){
                    return qmRex.exec(file.fileName);
                });

                const lprojDir = FileInfo.joinPaths(product.buildDirectory, product.bundle.bundleName,
                                                    "Contents", "Resources", "translations", locales[i] + product.bundle.localizedResourcesFolderSuffix);

                for (var j = 0; j < src.length; j++) {
                    data.push({
                        "source" : src[j].filePath,
                        "destination": FileInfo.joinPaths(lprojDir, src[j].fileName)
                    });
                }

                data.push({
                    "source" : FileInfo.joinPaths(project.sourceDirectory, "share", "translations", "Localizable.strings"),
                    "destination": FileInfo.joinPaths(lprojDir, "Localizable.strings")
                });

                const qtTranslationsMask = [
                    "qt_",
                    "qtbase_",
                    "qtxmlpatterns_"
                ];

                qtTranslationsMask.forEach(function(mask) {
                    var qmFile = FileInfo.joinPaths(product.i18n.qtTranslationsPath, mask + locales[i] + ".qm");
                    if (File.exists(qmFile)) {
                        data.push({
                            "source" : qmFile,
                            "destination": FileInfo.joinPaths(lprojDir, mask + locales[i] + ".qm")
                        });
                    } else {
                        const lang = locales[i].split('_')[0];
                        qmFile = FileInfo.joinPaths(product.i18n.qtTranslationsPath, mask + lang + ".qm");
                        if (File.exists(qmFile)) {
                            data.push({
                                "source" : qmFile,
                                "destination": FileInfo.joinPaths(lprojDir, mask + lang + ".qm")
                            });
                        }
                    }
                });
            }
            cmd.data = data;

            cmd.sourceCode = function() {
                data.forEach(function(copyData) {
                    File.copy(copyData.source, copyData.destination);
                });
            };
            return [cmd];
        }
    }

    windeployqt.noVirtualkeyboard: true

    Properties {
        condition: i18nconfig.limitDeploymentOfQtTranslations
        windeployqt.languages: i18nconfig.qtTranslationLocales.join(',')
    }

    Group {
        condition: qbs.targetOS.contains("macos") && bundle.isBundle
        fileTagsFilter: "bundle.content"
        qbs.install: true
        qbs.installDir: buildconfig.installAppPath
        qbs.installSourceBase: destinationDirectory
    }
}
