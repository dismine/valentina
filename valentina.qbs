import qbs.FileInfo

import "qbs/imports/conan/ConanfileProbe.qbs" as ConanfileProbe

Project {
    name: "Valentina"
    minimumQbsVersion: "1.22"
    qbsModuleProviders: ["Qt", "conan", "qbspkgconfig"]

    property bool enableConan: false
    property bool conanWithXerces: false
    property bool conanWithCrashReporting: false
    property string conanRemote
    property string minimumMacosVersion: undefined
    property string minimumQtVersion: "5.15"
    property stringList conanProfiles: []
    property bool enableSigning: true

    // Temporary probe until qbs doesn't support conan 2.0
    ConanfileProbe {
        id: thirdPartyConanPackages
        condition: enableConan && (conanWithXerces || conanWithCrashReporting)
        conanfilePath: project.sourceDirectory + "/conanfile.py"
        verbose: true
        profiles: conanProfiles
        remote: conanRemote
        options: {
            var o = {};
            if (conanWithXerces)
                o.with_xerces = "True";

            if (conanWithCrashReporting)
                o.with_crash_reporting = "True";
            return o;
        }
    }

    references: [
        "src/src.qbs",
        "share/translations/translations.qbs",
        "dmg.qbs",
        "winsetup.qbs"
    ]
    qbsSearchPaths: "qbs"

    AutotestRunner {
        Depends { name: "buildconfig" }

        arguments: ["-silent"]

        environment: {
            var env = base;

            if (qbs.targetOS.contains("unix") && !qbs.targetOS.contains("macos")) {
                env.push("LD_LIBRARY_PATH=" + qbs.installRoot + qbs.installPrefix + "/" + buildconfig.libDirName + "/valentina");
            }
            else if (qbs.targetOS.contains("windows")) {
                // PATH
                var path = "";
                for (var i = 0; i < env.length; ++i) {
                    if (env[i].startsWith("PATH=")) {
                        path = env[i].substring(5);
                        break;
                    }
                }

                var fullInstallDir = FileInfo.joinPaths(qbs.installRoot, qbs.installPrefix);
                if (path.length === 0) {
                    path = fullInstallDir;
                } else {
                    path = fullInstallDir + ";" + path;
                }

                var arrayElem = "PATH=" + path;
                if (i < env.length)
                    env[i] = arrayElem;
                else
                    env.push(arrayElem);

                // QT_QPA_PLATFORM_PLUGIN_PATH
                for (var i = 0; i < env.length; ++i) {
                    if (env[i].startsWith("QT_QPA_PLATFORM_PLUGIN_PATH=")) {
                        break;
                    }
                }

                if (i >= env.length) {
                    var pluginsPath = "QT_QPA_PLATFORM_PLUGIN_PATH=" + FileInfo.joinPaths(Qt.core.pluginPath, "platforms")
                    env.push(pluginsPath);
                }

                console.info("env_after: " + env);
            }
            return env;
        }
    }
}
