import qbs.FileInfo
import qbs.Utilities
import qbs.TemporaryDir

Project {
    InnoSetup {
        Depends { name: "buildconfig" }

        property bool _test: {
            var present = qbs.targetOS.includes("windows") && innosetup.present;
            console.info("has innosetup: " + present);
            if (present) {
                console.info("innosetup version " + innosetup.version);
            }
        }

        condition: qbs.targetOS.contains("windows")
        name: "ValentinaSetup"
        targetName: "ValentinaInstaller"
        version: buildconfig.projectVersion
        builtByDefault: false

        files: [
            project.sourceDirectory + "/dist/win/inno/valentina.iss"
        ]

        property string arhitecture: {
            if (qbs.architecture == "x86_64")
            {
                return "x64"
            }

            if (qbs.architecture == "arm64")
            {
                return "arm64";
            }

            return "";
        }

        property string minVersion: {
            if (Utilities.versionCompare(Qt.core.version, "6.5") >= 0)
            {
                return "10.0.17763";
            }

            if (Utilities.versionCompare(Qt.core.version, "6.3") >= 0)
            {
                return "10.0.19044";
            }

            if (Utilities.versionCompare(Qt.core.version, "6.2") >= 0)
            {
                return "10.0.17763";
            }

            if (Utilities.versionCompare(Qt.core.version, "6.0") >= 0)
            {
                return "10.0";
            }

            return "6.1";

        }

        innosetup.verboseOutput: true
        innosetup.defines: [
            "MyAppVersion=" + version,
            "MyAppCopyright=" + buildconfig.valentina_copyright_string,
            "MyAppArchitecture=" + arhitecture,
            "MyAppMinWinVersion=" + minVersion,
            "buildDirectory=" + project.qbs.installRoot
        ]
        innosetup.compilerFlags: ["/V9"]

        Group {
            fileTagsFilter: ['innosetup.exe']
            qbs.install: true
            qbs.installPrefix: '/'
        }
    }
}

