import qbs.File

Module {
    additionalProductTypes: ["multibundle"]

    property stringList targetApps: undefined

    Rule {
//        alwaysRun: true
        multiplex: true
        condition: product.qbs.targetOS.contains("macos") && product.buildconfig.enableMultiBundle && product.type.contains("dynamiclibrary")
        inputs: product.bundle.isBundle ? ["bundle.content"] : ["dynamiclibrary"]
        outputFileTags: ["multibundle"]
        outputArtifacts: {
            var artifactNames = [];

            const fileName = product.bundle.isBundle ? product.bundle.bundleName : inputs["dynamiclibrary"][0].fileName;

            const installRoot = product.qbs.installRoot + product.qbs.installPrefix + "/" + product.buildconfig.installAppPath;
            product.multibundle.targetApps.forEach(function(targetApp) {
                artifactNames.push(installRoot + "/" + targetApp + ".app/Contents/Frameworks/" + fileName);

                if (product.installDebugInformation)
                    artifactNames.push(installRoot + "/" + targetApp + ".app/Contents/Frameworks/" + fileName +
                                       product.cpp.debugInfoBundleSuffix);
            });

            var artifacts = artifactNames.map(function(art){
                var a = {
                    filePath: art,
                    fileTags: ["multibundle"]
                }
                return a;
            });
            return artifacts;
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Copying dynamic library into bundles";
            cmd.highlight = "filegen";

            const fileName = product.bundle.isBundle ? product.bundle.bundleName : inputs["dynamiclibrary"][0].fileName;
            const installRoot = product.qbs.installRoot + product.qbs.installPrefix + "/" + product.buildconfig.installAppPath;
            var data = [];
            product.multibundle.targetApps.forEach(function(targetApp) {
                data.push({
                    "source" : product.buildDirectory + "/" + fileName,
                    "destination": installRoot + "/" + targetApp + ".app/Contents/Frameworks/" + fileName
                });

                if (product.installDebugInformation)
                    data.push({
                        "source" : product.buildDirectory + "/" + fileName + product.cpp.debugInfoBundleSuffix,
                        "destination": installRoot + "/" + targetApp + ".app/Contents/Frameworks/" + fileName +
                                       product.cpp.debugInfoBundleSuffix
                    });
            });

            cmd.data = data;

            cmd.sourceCode = function() {
                data.forEach(function(copyData) {
                    File.copy(copyData.source, copyData.destination);
                });
            };
            return [cmd];
        }
    }

    Rule {
//        alwaysRun: true
        condition: product.qbs.targetOS.contains("macos") && !product.buildconfig.enableMultiBundle && product.type.contains("application")
        inputs: ["bundle.application-executable"]
        outputFileTags: ["multibundle"]
        outputArtifacts: {
            var artifactNames = [];

            const installRoot = product.qbs.installRoot + product.qbs.installPrefix + "/" + product.buildconfig.installAppPath;
            product.multibundle.targetApps.forEach(function(targetApp) {
                artifactNames.push(installRoot + "/" + targetApp + ".app/Contents/MacOS/" + input.fileName);

                if (product.installDebugInformation)
                    artifactNames.push(installRoot + "/" + targetApp + ".app/Contents/MacOS/" + input.fileName +
                                       product.cpp.debugInfoBundleSuffix);
            });

            var artifacts = artifactNames.map(function(art){
                var a = {
                    filePath: art,
                    fileTags: ["multibundle"]
                }
                return a;
            });
            return artifacts;
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Copying auxiliary binary into bundle";
            cmd.highlight = "filegen";

            const fileName = product.bundle.isBundle ? product.bundle.bundleName : inputs["application"][0].fileName;
            const installRoot = product.qbs.installRoot + product.qbs.installPrefix + "/" + product.buildconfig.installAppPath;
            var data = [];
            product.multibundle.targetApps.forEach(function(targetApp) {
                data.push({
                    "source" : input.filePath,
                    "destination": installRoot + "/" + targetApp + ".app/Contents/MacOS/" + input.fileName
                });

                if (product.installDebugInformation)
                    data.push({
                        "source" : product.buildDirectory + "/" + input.fileName + product.cpp.debugInfoBundleSuffix,
                        "destination": installRoot + "/" + targetApp + ".app/Contents/MacOS/" + fileName +
                                       product.cpp.debugInfoBundleSuffix
                    });
            });

            cmd.data = data;

            cmd.sourceCode = function() {
                data.forEach(function(copyData) {
                    console.info("Dynamic source: " + copyData.source);
                    console.info("Dynamic destination: " + copyData.destination);
                    File.copy(copyData.source, copyData.destination);
                });
            };
            return [cmd];
        }
    }
}
