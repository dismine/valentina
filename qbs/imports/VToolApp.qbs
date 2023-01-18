import qbs.FileInfo

VApp {
    Depends { name: "freedesktop2" }
    Depends { name: "tenv" }
    Depends { name: "windeployqt"; }
    Depends { name: "i18nconfig"; }

    version: "0.7.52"
    install: true
    installDir: buildconfig.installAppPath
    installDebugInformation: true
    consoleApplication: false

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
                files.push("measurements_" + pmSystems[i] + ".ts");
                for (var j = 0; j < locales.length; j++) {
                    files.push("measurements_" + pmSystems[i] + "_" + locales[j] + ".ts");
                }
            }

            return files;
        }
    }

    windeployqt.noVirtualkeyboard: true

    Properties {
        condition: i18nconfig.limitDeploymentOfQtTranslations
        windeployqt.languages: i18nconfig.qtTranslationLocales.join(',')
    }
}
