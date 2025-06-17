/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qbs.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms and
** conditions see http://www.qt.io/terms-conditions. For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

import qbs.File
import qbs.FileInfo
import qbs.ModUtils

AppleDiskImage {
    property string sourceBase: "/Applications"
    readonly property string absoluteSourceBase: FileInfo.joinPaths(project.qbs.installRoot, project.qbs.installPrefix, sourceBase)
    property stringList symlinks: ["/Applications:Applications"]

    readonly property string stageDirectory: FileInfo.joinPaths(destinationDirectory, "Volumes", dmg.volumeName)

    type: base.concat("dmg.dummy.signed")

    Rule {
        multiplex: true
        outputFileTags: ["dmg.input", "dmg.input.symlink"]
        outputArtifacts: Array.prototype.map.call(product.symlinks, function (symlink) {
            var symlinkTarget = symlink.split(':')[0];
            var symlinkName = symlink.split(':')[1] || symlinkTarget;
            if (FileInfo.isAbsolutePath(symlinkName))
                throw(symlink + " is an invalid symlink; the destination must be a relative path");
            return {
                filePath: FileInfo.joinPaths(product.stageDirectory, symlinkName),
                fileTags: ["dmg.input", "dmg.input.symlink"],
                dmg: { symlinkTarget: symlinkTarget, sourceBase: product.stageDirectory },
            };
        })
        prepare: Array.prototype.map.call(outputs["dmg.input"], function (symlink) {
            var cmd = new Command("ln", ["-sfn", symlink.dmg.symlinkTarget, symlink.filePath]);
            cmd.workingDirectory = product.stageDirectory;
            cmd.description = "symlinking " + symlink.fileName + " to " + symlink.dmg.symlinkTarget;
            return cmd;
        });
    }

    Rule {
        multiplex: true
        inputs: ["dmg.input.symlink", "dmg.input.app"]
        outputFileTags: ["dmg.input"]
        outputArtifacts: {
            var absSourceBase = product.absoluteSourceBase;
            var symlinkPaths = (inputs["dmg.input.symlink"] || []).map(function (s) { return s.filePath; });
            return Array.prototype.map.call(inputs["dmg.input.app"], function (a) {
                var fp = a.filePath;
                if (fp.startsWith(absSourceBase)) {
                    var outputFilePath = fp.replace(absSourceBase, product.stageDirectory);

                    // Check for symlink conflicts
                    for (var i in symlinkPaths) {
                        if (outputFilePath.startsWith(symlinkPaths[i]))
                            throw new Error("Cannot install '" + a.filePath
                                + "' to '" + outputFilePath + "' because it "
                                + "would conflict with the symlink at '"
                                + symlinkPaths[i] + "'");
                    }

                    return {
                        filePath: outputFilePath,
                        fileTags: ["dmg.input"],
                        dmg: { sourceBase: product.stageDirectory }
                    }
                }
            }).filter(function (a) { return !!a; });
        }
        prepare: {
            var cmds = [], dmgs = outputs["dmg.input"];
            for (var i in dmgs) {
                var a = dmgs[i];
                var src = a.filePath.replace(product.stageDirectory, product.absoluteSourceBase);
                var dst = a.filePath;
                var cmd = new Command("cp", ["-R", src, dst]);
                cmd.description = "copying " + a.fileName;
                cmds.push(cmd);
            }
            return cmds;
        }
    }

    Rule {
        multiplex: true
        inputs: ["dmg.dmg"]
        Artifact {
            fileTags: ["dmg.dummy.signed"]
            filePath: FileInfo.joinPaths(product.destinationDirectory, "dummy.txt")
        }
        prepare: {
            var cmds = [];

            var enableCodeSigning = product.codesign.enableCodeSigning;
            if (enableCodeSigning) {
                var actualSigningIdentity = product.codesign._actualSigningIdentity;
                if (!actualSigningIdentity) {
                    throw "No codesigning identities (i.e. certificate and provate key pairs) matching '"
                          + product.codesign.signingIdentity + "' were found.";
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

                args.push(input.filePath);
                var cmdSign = new Command(product.codesign.codesignPath, args);
                cmdSign.description = "codesign " + input.fileName
                        + " (" + actualSigningIdentity.subjectInfo.CN + ")";
                cmdSign.stdoutFilePath = output.filePath;
                cmdSign.inputFilePath = input.filePath;
                cmdSign.stderrFilterFunction = function(stderr) {
                    return stderr.replace(inputFilePath + ": replacing existing signature\n", "");
                };

                cmds.push(cmdSign);
            }

            var cmdCp = new JavaScriptCommand();
            cmdCp.description = "installing " + input.fileName;
            cmdCp.highlight = "codegen";
            cmdCp.src = input.filePath;
            cmdCp.dst = FileInfo.joinPaths(product.qbs.installRoot, "share", input.fileName);
            cmdCp.sourceCode = function() {
                console.warn("src: " + src);
                console.warn("dst: " + dst);
                File.copy(src, dst);
            }
            cmds.push(cmdCp);

            return cmds;
        }
    }
}
