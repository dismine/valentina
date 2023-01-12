import qbs.FileInfo

Product {
    Depends { name: "i18n" }

    name: "MTranslations"
    type: "ts"
    builtByDefault: false

    Group {
        name: "Headers"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "src", FileInfo.pathSeparator())
        files: [
            "libs/vpatterndb/vtranslatemeasurements.h"
        ]
        fileTags: "i18n.hpp"
    }

    Group {
        name: "Sources"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "src", FileInfo.pathSeparator())
        files: [
            "libs/vpatterndb/vtranslatemeasurements.cpp"
        ]
        fileTags: "i18n.src"
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

        var pmSystems = [
            "p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15",
            "p16", "p17", "p18", "p19", "p20", "p21", "p22", "p23", "p24", "p25", "p26", "p27", "p28", "p29",
            "p30", "p31", "p32", "p33", "p34", "p35", "p36", "p37", "p38", "p39", "p40", "p41", "p42", "p43",
            "p44", "p45", "p46", "p47", "p48", "p49", "p50", "p51", "p52", "p53", "p54", "p998"
        ]

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
