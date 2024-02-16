import qbs.FileInfo
import qbs.Utilities

Module {
    condition: (qbs.debugInformation && qbs.toolchain.contains("gcc") && !qbs.toolchain.contains("clang")) ||
               qbs.toolchain.contains("clang")
    Depends { name: "cpp" }
    additionalProductTypes: ["gcno"]

    cpp.driverFlags: {
        var flags = [];

        if (product.buildconfig.enableUnitTests && product.buildconfig.enableTestCoverage) {
            if (qbs.toolchain.contains("clang"))
                flags.push("-fprofile-instr-generate", "-fcoverage-mapping");
            else
                flags.push("--coverage");
        }

        return flags;
    }

    Rule { // Fake rule for '*.gcno' generation.
        condition: qbs.debugInformation && qbs.toolchain.contains("gcc") && !qbs.toolchain.contains("clang")
        inputs: ["cpp", "c"]
        outputFileTags: ["gcno"]
        outputArtifacts: {
            return [{
                fileTags: ["gcno"],
                filePath: FileInfo.joinPaths(Utilities.getHash(input.baseDir),
                                             input.fileName + ".gcno")
            }];
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "generating " + output.fileName;
            return [cmd];
        }
    }
}
