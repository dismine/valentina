import qbs.Probes

Project {
    name: "Valentina"
    minimumQbsVersion: "1.16"

    property bool enableConan: false
    property string minimumMacosVersion: undefined

    Probes.ConanfileProbe {
        id: thirdPartyConanPackages
        condition: enableConan
        conanfilePath: project.sourceDirectory + "/conanfile.py"
        settings: {
            if (qbs.targetOS.contains("macos") && project.minimumMacosVersion !== undefined)
                return ({"os.version": project.minimumMacosVersion});
            return undefined;
        }
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
