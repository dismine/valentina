import qbs.FileInfo

Product {
    Depends { name: "i18n" }
    Depends { name: "i18nconfig" }

    name: "MTranslations"
    type: "ts"
    builtByDefault: false

    Group {
        name: "Headers"
        prefix: project.sourceDirectory + "/src/"
        files: [
            "libs/vpatterndb/vtranslatemeasurements.h"
        ]
        fileTags: "i18n.hpp"
    }

    Group {
        name: "Sources"
        prefix: project.sourceDirectory + "/src/"
        files: [
            "libs/vpatterndb/vtranslatemeasurements.cpp"
        ]
        fileTags: "i18n.src"
    }

    Group {
        name: "Translations"
        files: {
            var files = [];

        var locales = i18nconfig.translationLocales;
        var pmSystems = i18nconfig.pmSystems;

        for (var i = 0; i < pmSystems.length; i++) {
            files.push("measurements_" + pmSystems[i] + ".ts");
            for (var j = 0; j < locales.length; j++) {
                files.push("measurements_" + pmSystems[i] + "_" + locales[j] + ".ts");
            }
        }

            return files;
        }
        fileTags: "i18n.ts"
    }
}
