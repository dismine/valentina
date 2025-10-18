import qbs.FileInfo
import "rcc.js" as Rcc

Module {

    additionalProductTypes: "ebr.rcc"

    property bool enableCompression: true
    property int thresholdLevel: 70
    property string compressAlgorithm: "zstd"
    property int compressLevel: -1

    Scanner {
        inputs: ["ebr.external_qrc"]
        searchPaths: [FileInfo.path(input.filePath)]
        scan: Rcc.scanQrc(product, input.filePath)
    }

    Rule {
        inputs: ["ebr.external_qrc"]
        outputFileTags: ["ebr.rcc"]
        outputArtifacts: {
            var artifact = {
                filePath: input.completeBaseName + ".rcc",
                fileTags: ["ebr.rcc"]
            };
            return [artifact];
        }
        prepare: {
            var args = ["-binary", input.filePath, "-o", output.filePath];
            var enableCompression = input.moduleProperty("ebr", "enableCompression");
            if (enableCompression) {
                var compressAlgorithm = input.moduleProperty("ebr", "compressAlgorithm");
                if (compressAlgorithm !== "zstd")
                    args.push("-compress-algo", compressAlgorithm);

                var thresholdLevel = input.moduleProperty("ebr", "thresholdLevel");
                if (thresholdLevel !== 70)
                    args.push("-threshold", thresholdLevel);

                var compressLevel = input.moduleProperty("ebr", "compressLevel");
                if (compressLevel !== -1) {
                    // rcc will silently ignore incorrect values
                    if (compressAlgorithm === "zstd")
                        compressLevel = Rcc.bound(0, compressLevel, 19);
                    else if (compressAlgorithm === "zlib")
                        compressLevel = Rcc.bound(1, compressLevel, 9);
                }

                if (compressLevel !== -1)
                    args.push("-compress", compressLevel);
            } else {
                args.push("-no-compress");
            }

            var cmd = new Command(Rcc.fullPath(product), args);
            cmd.description = "rcc (external) "+ input.fileName;
            cmd.highlight = 'filegen';
            return cmd;
        }
    }
}
