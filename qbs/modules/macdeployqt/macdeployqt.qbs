import qbs.File
import qbs.FileInfo
import qbs.Environment
import qbs.Utilities
import qbs.ModUtils

/**
  This module uses _macdeployqt_ program to collect Qt runtime for MacOS deployment.
  */
Module {
    additionalProductTypes: ["macdeployqt"]

    property bool artifacts: true

    property string macdeployqtArtifact: artifacts ? "maceployqt.txt" : undefined

    property string macdeployqtProgramBinPath : undefined

    property string macdeployqtProgram: "macdeployqt"

    property bool noPlugins: false

    property int verbose: undefined

    property bool noStrip: true

    property bool useDebugLibs: false

    property bool alwaysOverwrite: false

    property bool appstoreCompliant: false

    property stringList libpath: undefined

    property string pluginspath: undefined

    property stringList targetApps: undefined

    property bool signForNotarization: false

    property string signingIdentity: "-" // ad-hoc

    Rule {
//        alwaysRun: true
        multiplex: true
        condition: product.qbs.targetOS.contains("macos") && product.bundle.isBundle && (product.buildconfig.enableMultiBundle || (!product.buildconfig.enableMultiBundle && product.primaryApp))
        inputs: ["bundle.content"]

        prepare: {
            var macdeployqtProgram = product.macdeployqt.macdeployqtProgram;

            // Checking if directory exists as a dirty workaround to check if `--no-install` options has been set from command line.

            if (!File.exists(product.qbs.installRoot)) {
                var cmd = new JavaScriptCommand();
                cmd.macdeployqtProgram = macdeployqtProgram;
                cmd.description = "can not invoke '" + macdeployqtProgram + "' when '--no-install` options has been set from command line";
                cmd.sourceCode = function () {
                    console.warn("Can not invoke '" + macdeployqtProgram + "' when '--no-install` options has been set from command line (tip: remove your build directory entirely after unchecking '--no-install' option)");
                }
            } else {
                const installRoot = product.qbs.installRoot + product.qbs.installPrefix + "/" + product.buildconfig.installAppPath;

                var cmdArgs = [installRoot + "/" + product.targetName + ".app"];

                if (product.macdeployqt.noPlugins)
                    cmdArgs.push("-no-plugins");

                if (product.macdeployqt.verbose !== undefined)
                    cmdArgs.push("-verbose=" + product.macdeployqt.verbose);

                if (product.macdeployqt.noStrip)
                    cmdArgs.push("-no-strip");

                if (product.macdeployqt.useDebugLibs)
                    cmdArgs.push("-use-debug-libs");

                if (product.macdeployqt.alwaysOverwrite)
                    cmdArgs.push("-always-overwrite");

                if (product.macdeployqt.appstoreCompliant)
                    cmdArgs.push("-appstore-compliant");

                if (product.macdeployqt.libpath !== undefined)
                    product.macdeployqt.libpath.forEach(function(libpath) {
                        cmdArgs.push("-libpath=" + libpath);
                    });

                if (product.macdeployqt.pluginspath !== undefined)
                    cmdArgs.push("-pluginspath=" + product.macdeployqt.pluginspath);

                if (product.buildconfig.enableCodeSigning) {
                    if (product.macdeployqt.signForNotarization)
                        cmdArgs.push("-sign-for-notarization=" + product.macdeployqt.signingIdentity);
                    else
                        cmdArgs.push("-codesign=" + product.macdeployqt.signingIdentity);
                }

                if (product.macdeployqt.targetApps !== undefined)
                {
                    product.macdeployqt.targetApps.forEach(function(targetApp) {
                        cmdArgs.push("-executable=" + FileInfo.joinPaths(installRoot, product.targetName + ".app", "Contents", "MacOS", targetApp));
                    });
                }

                var macdeployqtProgramBinPath = product.Qt.core.binPath;
                if (product.macdeployqt.macdeployqtProgramBinPath !== undefined)
                    macdeployqtProgramBinPath = product.macdeployqt.macdeployqtProgramBinPath;

                var cmd = new Command(macdeployqtProgramBinPath + "/" + macdeployqtProgram, cmdArgs);
                cmd.jobPool = "macdeployqt";
                cmd.description = "invoking '" + macdeployqtProgram;
                cmd.stdoutFilePath = product.buildDirectory + "/" + product.macdeployqt.macdeployqtArtifact;
            }
            cmd.workingDirectory = product.qbs.installRoot;
            cmd.highlight = "filegen";
            return [cmd]
        }

        Artifact {
            filePath: product.buildDirectory + "/" + product.macdeployqt.macdeployqtArtifact
            fileTags: ["macdeployqt"]
        }
    }

    JobLimit {
        jobPool: "macdeployqt"
        jobCount: 1
    }
}
