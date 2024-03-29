import qbs.File
import qbs.FileInfo
import qbs.Process
import qbs.TextFile
import qbs.Utilities

Module {
    property string type: typeProbe.type
    property string repoDir: project.sourceDirectory

    // TODO: If minimal qbs version is 1.23 replace with FileInfo.executableSuffix()
    readonly property string executableSuffix: project.qbs.targetOS.contains("windows") ? ".exe" : ""
    property string toolFilePath: {
        if (type === "git")
            return "git" + executableSuffix;
        if (type === "svn")
            return "svn" + executableSuffix;
    }

    property string headerFileName: "vcs-repo-state.h"
    readonly property string repoState: gitProbe.repoState || subversionProbe.repoState
    readonly property string repoStateTag: gitProbe.repoStateTag || subversionProbe.repoStateTag
    readonly property string repoStateDistance: gitProbe.repoStateDistance || subversionProbe.repoStateDistance
    readonly property string repoStateRevision: gitProbe.repoStateRevision || subversionProbe.repoStateRevision

    // Internal
    readonly property string includeDir: FileInfo.joinPaths(product.buildDirectory, "vcs-include")
    readonly property string metaDataBaseDir: typeProbe.metaDataBaseDir

    PropertyOptions {
        name: "type"
        allowedValues: ["git", "svn"]
        description: "the version control system your project is using"
    }

    Depends { name: "cpp"; condition: headerFileName }
    Properties {
        condition: headerFileName
        cpp.includePaths: [includeDir]
    }

    Probe {
        id: typeProbe

        property string tool: toolFilePath
        property string theRepoDir: repoDir

        property string type
        property string metaDataBaseDir

        configure: {
            var detector = new Process();
            try {
                detector.setWorkingDirectory(theRepoDir);
                if (detector.exec(tool || "git", ["rev-parse", "--git-dir"]) === 0) {
                    found = true;
                    type = "git";
                    metaDataBaseDir = detector.readStdOut().trim();
                    if (!FileInfo.isAbsolutePath(metaDataBaseDir))
                        metaDataBaseDir = FileInfo.joinPaths(theRepoDir, metaDataBaseDir);
                    return;
                }
                if (detector.exec(tool || "svn",
                                  ["info", "--show-item", "wc-root", "--no-newline"]) === 0) {
                    found = true
                    type = "svn";
                    metaDataBaseDir = FileInfo.joinPaths(detector.readStdOut(), ".svn");
                    return;
                } else if (detector.exec(tool || "svn", ["info"]) === 0) {
                    if (detector.exec(tool || "svn", ["--version", "--quiet"]) === 0
                            && Utilities.versionCompare(detector.readStdOut().trim(), "1.9") < 0) {
                        throw "svn too old, version >= 1.9 required";
                    }
                }
            } finally {
                detector.close();
            }
        }
    }

    Probe {
        id: gitProbe
        condition: type === "git"

        property string tool: toolFilePath
        property string theRepoDir: repoDir

        property string repoState
        property string repoStateTag
        property string repoStateDistance
        property string repoStateRevision

        configure: {
            var commitsProbe = new Process();
            try {
                commitsProbe.setWorkingDirectory(theRepoDir);
                var exitCode = commitsProbe.exec(tool, ["rev-list", "HEAD", "--count"], false);
                if (exitCode !== 0) {
                    console.info("Cannot read repo state.");
                    return;
                }

                var count = parseInt(commitsProbe.readStdOut().trim());
                if (count < 1) {
                    console.info("The repo has no commits yet.");
                    return;
                }
            } finally {
                commitsProbe.close();
            }

            var proc = new Process();
            try {
                proc.setWorkingDirectory(theRepoDir);
                // tag is formatted as TAG-N-gSHA:
                // 1. latest stable version is TAG, or vX.Y.Z
                // 2. number of commits since latest stable version is N
                // 3. latest commit is gSHA
                proc.exec(tool, ["describe", "--always", "HEAD"], true);
                repoState = proc.readStdOut().trim();
                if (repoState) {
                    found = true;

                    const tagSections = repoState.split("-");

                    if (tagSections.length >= 3) {
                        repoStateTag = tagSections[0];
                        repoStateDistance = tagSections[1];
                        repoStateRevision = tagSections[2];
                    } else  {
                        repoStateRevision = tagSections[0];
                    }
                }
            } finally {
                proc.close();
            }
        }
    }

    Probe {
        id: subversionProbe
        condition: type === "svn"

        property string tool: toolFilePath
        property string theRepoDir: repoDir
        property string filePath: FileInfo.joinPaths(metaDataBaseDir, "wc.db")
        property var timestamp: File.lastModified(filePath)

        property string repoState
        property string repoStateTag
        property string repoStateDistance
        property string repoStateRevision

        configure: {
            var proc = new Process();
            try {
                proc.setWorkingDirectory(theRepoDir);
                proc.exec(tool, ["info", "-r", "HEAD", "--show-item", "revision", "--no-newline"],
                          true);
                repoState = proc.readStdOut().trim();
                if (repoState)
                    found = true;
            } finally {
                proc.close();
            }
        }
    }

    Rule {
        condition: headerFileName
        multiplex: true
        Artifact {
            filePath: FileInfo.joinPaths(product.vcs2.includeDir, product.vcs2.headerFileName)
            fileTags: ["hpp"]
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "generating " + output.fileName;
            cmd.highlight = "codegen";
            cmd.repoState = product.vcs2.repoState;
            cmd.repoStateTag = product.vcs2.repoStateTag;
            cmd.repoStateDistance = product.vcs2.repoStateDistance;
            cmd.repoStateRevision = product.vcs2.repoStateRevision;
            cmd.sourceCode = function() {
                var f = new TextFile(output.filePath, TextFile.WriteOnly);
                try {
                    f.writeLine("#ifndef VCS_REPO_STATE_H");
                    f.writeLine("#define VCS_REPO_STATE_H");
                    f.writeLine('#define VCS_REPO_STATE "' + (repoState ? repoState : "none") + '"')
                    f.writeLine('#define VCS_REPO_STATE_TAG "' + (repoStateTag ? repoStateTag : "none") + '"')
                    f.writeLine('#define VCS_REPO_STATE_DISTANCE "' + (repoStateDistance ? repoStateDistance : "none") + '"')
                    f.writeLine('#define VCS_REPO_STATE_REVISION "' + (repoStateRevision ? repoStateRevision : "none") + '"')
                    f.writeLine("#endif");
                } finally {
                    f.close();
                }
            };
            return [cmd];
        }
    }
}
