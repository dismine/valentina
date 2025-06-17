import qbs.FileInfo

Product {
    Depends { name: "i18n" }
    Depends { name: "i18nconfig" }

    name: "Translations"
    type: "ts"
    builtByDefault: false

    Group {
        name: "Headers"
        prefix: project.sourceDirectory + "/src/"
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
        prefix: project.sourceDirectory + "/src/"
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
        prefix: project.sourceDirectory + "/src/"
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

            var locales = i18nconfig.translationLocales;

            files.push("valentina.ts");

            for (var i = 0; i < locales.length; i++) {
                files.push("valentina_" + locales[i] + ".ts");
            }

            return files;
        }
        fileTags: "i18n.ts"
    }
}
