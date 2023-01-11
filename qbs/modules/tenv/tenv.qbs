import qbs.FileInfo
import qbs.File

Module {
    additionalProductTypes: "tenv.deps"

    // Use this property to preparing test environment. By default for debug purposes we copy some nessesary files and
    // libraries into build directory.
    property bool enableTestEnvironment: true

    Rule {
        multiplex: true
//        alwaysRun: true
        condition: enableTestEnvironment
        inputs: ["pdftops_dist_macx", "pdftops_dist_win", "openssl_dist", "multisize_tables", "label_templates"]
        outputFileTags: ["tenv.deps"]
        outputArtifacts: {
            var artifactNames = (inputs["pdftops_dist_macx"] || []).map(function(file){
                return FileInfo.joinPaths(product.buildDirectory, file.fileName);
            });

            artifactNames = artifactNames.concat((inputs["pdftops_dist_win"] || []).map(function(file){
                return FileInfo.joinPaths(product.buildDirectory, file.fileName);
            }));

            artifactNames = artifactNames.concat((inputs["openssl_dist"] || []).map(function(file){
                return FileInfo.joinPaths(product.buildDirectory, file.fileName);
            }));

            artifactNames = artifactNames.concat((inputs["multisize_tables"] || []).map(function(file){
                return FileInfo.joinPaths(product.buildDirectory, "tables", "multisize",file.fileName);
            }));

            artifactNames = artifactNames.concat((inputs["label_templates"] || []).map(function(file){
                return FileInfo.joinPaths(product.buildDirectory, "labels",file.fileName);
            }));

            var artifacts = artifactNames.map(function(art){
                var a = {
                    filePath: art,
                    fileTags: ["tenv.deps"]
                }
                return a;
            });
            return artifacts;
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Preparing test environment";
            cmd.highlight = "filegen";

            var sources = (inputs["pdftops_dist_macx"] || []).map(function(artifact) {
                return artifact.filePath;
            });

            sources = sources.concat((inputs["pdftops_dist_win"] || []).map(function(artifact) {
                return artifact.filePath;
            }));

            sources = sources.concat((inputs["openssl_dist"] || []).map(function(artifact) {
                return artifact.filePath;
            }));

            sources = sources.concat((inputs["multisize_tables"] || []).map(function(artifact) {
                return artifact.filePath;
            }));

            sources = sources.concat((inputs["label_templates"] || []).map(function(artifact) {
                return artifact.filePath;
            }));

            cmd.sources = sources;

            var destination = (inputs["pdftops_dist_macx"] || []).map(function(artifact) {
                return FileInfo.joinPaths(product.buildDirectory, artifact.fileName);
            });

            destination = destination.concat((inputs["pdftops_dist_win"] || []).map(function(artifact) {
                return FileInfo.joinPaths(product.buildDirectory, artifact.fileName);
            }));

            destination = destination.concat((inputs["openssl_dist"] || []).map(function(artifact) {
                return FileInfo.joinPaths(product.buildDirectory, artifact.fileName);
            }));

            destination = destination.concat((inputs["multisize_tables"] || []).map(function(artifact) {
                return FileInfo.joinPaths(product.buildDirectory, "tables", "multisize", artifact.fileName);
            }));

            destination = destination.concat((inputs["label_templates"] || []).map(function(artifact) {
                return FileInfo.joinPaths(product.buildDirectory, "labels", artifact.fileName);
            }));

            cmd.destination = destination;
            cmd.sourceCode = function() {
                for (var i in sources) {
                    File.copy(sources[i], destination[i]);
                }
            };
            return [cmd];
        }
    }
}
