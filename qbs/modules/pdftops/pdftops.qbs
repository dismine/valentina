import qbs.Process
import qbs.File
import qbs.FileInfo
import qbs.Utilities

Module {

    additionalProductTypes: ["pdftops.bundled"]

    readonly property string pdftopsPath: pdftopsProbe.path
    readonly property bool pdftopsPresent: pdftopsProbe.found

	Probe {
        id: pdftopsProbe

        property string path
        property string sourceDirectory: project.sourceDirectory
        property string qtVersion: product.Qt.core.version
        property string architecture: product.qbs.architecture

        configure: {
            if (Utilities.versionCompare(qtVersion, "6") >= 0) {
                var detector = new Process();
                try {
                    if (detector.exec("which", ["pdftops"]) === 0) {
                        var binPath = detector.readStdOut().trim(); // we can read only one time
                        if (detector.exec("realpath", [binPath]) === 0) {
                            path = detector.readStdOut().trim();
                            console.info("Found pdftops at '" + path + "'.");
                            found = true;
                        }
                    }
                } finally {
                    detector.close();
                }
            } else {
                var binPath = sourceDirectory + "/dist/macx/bin64/pdftops";
                if (architecture.contains("x86_64") && File.exists(binPath)) {
                    path = binPath;
                    found = true;
                }
            }
        }
    }

    Rule {
        multiplex: true
        inputs: ["pdftops.in"]

        prepare: {
            var cmds = [];

            var src = input.filePath;
            var dst = output.filePath;
            var copyCmd = new Command("cp", [src, dst]);
            copyCmd.highlight = 'filegen';
            copyCmd.description = "copying " + input.fileName;
            cmds.push(copyCmd);

            if (!product.codesign.enableCodeSigning)
                return cmds;

            var actualSigningIdentity = product.codesign._actualSigningIdentity;
            if (!actualSigningIdentity) {
                throw "No codesigning identities (i.e. certificate and private key pairs) matching “"
                        + product.codesign.signingIdentity + "” were found.";
            }

            var args = ["--force", "--sign", actualSigningIdentity.SHA1];

            // If signingTimestamp is undefined or empty, do not specify the flag at all -
            // this uses the system-specific default behavior
            var signingTimestamp = product.codesign.signingTimestamp;
            if (signingTimestamp) {
                // If signingTimestamp is an empty string, specify the flag but do
                // not specify a value - this uses a default Apple-provided server
                var flag = "--timestamp";
                if (signingTimestamp)
                    flag += "=" + signingTimestamp;
                args.push(flag);
            }

            args = args.concat(product.codesign.codesignFlags || []);

            args.push(output.filePath);
            console.warn(args);
            var cmdSign = new Command(product.codesign.codesignPath, args);
            cmdSign.description = "codesign " + output.fileName
                    + " (" + actualSigningIdentity.subjectInfo.CN + ")";
            cmdSign.outputFilePath = output.filePath;
            cmdSign.stderrFilterFunction = function(stderr) {
                return stderr.replace(outputFilePath + ": replacing existing signature\n", "");
            };
            cmds.push(cmdSign);

            return cmds;
        }

        Artifact {
            filePath: FileInfo.joinPaths(product.qbs.installRoot, product.qbs.installPrefix,
                                         product.buildconfig.installAppPath,
                                         product.targetName + ".app", "Contents", "MacOS", "pdftops")
            fileTags: ["pdftops.bundled"]
        }
    }
}

