import "qbs/imports/conan/ConanfileProbe.qbs" as ConanfileProbe

Project {
    name: "Valentina"
    minimumQbsVersion: "1.16"

    property bool enableConan: false
    property string minimumMacosVersion: undefined
    property stringList conanProfiles: []

    // Temporary probe until qbs doesn't support conan 2.0
    ConanfileProbe {
        id: thirdPartyConanPackages
        condition: enableConan
        conanfilePath: project.sourceDirectory + "/conanfile.py"
        verbose: true
        profiles: conanProfiles
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
