import qbs.FileInfo

Product {
    Depends { name: "i18n" }

    name: "Translations"
    type: "ts"
    builtByDefault: false

    Group {
        name: "Headers"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "src", FileInfo.pathSeparator())
        files: [
            "app/**/*.h",
            "app/**/*.hpp",
            "libs/**/*.h",
            "libs/**/*.hpp"
        ]
        excludeFiles: [
            "libs/vpatterndb/vtranslatemeasurements.h"
        ]
        fileTags: "i18n.hpp"
    }

    Group {
        name: "Sources"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "src", FileInfo.pathSeparator())
        files: [
            "app/**/*.cpp",
            "app/**/*.js",
            "app/**/*.qml",
            "libs/**/*.cpp",
            "libs/**/*.js",
            "libs/**/*.qml"
        ]
        excludeFiles: [
            "libs/vpatterndb/vtranslatemeasurements.cpp"
        ]
        fileTags: "i18n.src"
    }

    Group {
        name: "Forms"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "src", FileInfo.pathSeparator())
        files: [
            "app/**/*.ui",
            "libs/**/*.ui"
        ]
        fileTags: "i18n.ui"
    }

    Group {
        name: "Translations"
        files: {
            var files = [];

            var locales = [
                "uk_UA",
                "de_DE",
                "cs_CZ",
                "he_IL",
                "fr_FR",
                "it_IT",
                "nl_NL",
                "id_ID",
                "es_ES",
                "fi_FI",
                "en_US",
                "en_CA",
                "en_IN",
                "ro_RO",
                "zh_CN",
                "pt_BR",
                "el_GR",
                "pl_PL"
            ];

            files.push("valentina.ts");

            for (var i = 0; i < locales.length; i++) {
                files.push("valentina_" + locales[i] + ".ts");
            }

            return files;
        }
        fileTags: "i18n.ts"
    }
}
