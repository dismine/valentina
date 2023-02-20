import qbs.Probes

Project {
    name: "Valentina"
    minimumQbsVersion: "1.16"

    property bool enableConan: false

    Probes.ConanfileProbe {
        id: thirdPartyConanPackages
        condition: enableConan
        conanfilePath: project.sourceDirectory + "/conanfile.txt"
    }

    references: [
        "src/src.qbs",
        "share/translations/translations.qbs",
        "share/translations/measurements.qbs",
    ]
    qbsSearchPaths: "qbs"

    AutotestRunner {
        Depends { name: "buildconfig" }

        environment: {
            var env = base;

            if (qbs.targetOS.contains("unix") && !qbs.targetOS.contains("macos")) {
                env.push("LD_LIBRARY_PATH=" + qbs.installRoot + qbs.installPrefix + "/" + buildconfig.libDirName + "/valentina");
            }
            return env;
        }
    }
}
