/****************************************************************************
**
** Copyright (C) 2020 Richard Weickelt
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

import qbs.Process
import qbs.File
import qbs.FileInfo
import qbs.TextFile
import qbs.Utilities

Probe {
    // Inputs
    property stringList additionalArguments: []
    property path conanfilePath
    property path packageReference
    property path executable: "conan"
    property stringList generators: ["json"]
    property var options
    property var settings
    property bool verbose: false
    property stringList profiles: []
    property string remote

    // Output
    property var dependencies
    property path generatedFilesPath
    property var json

    // Internal
    // Ensure that the probe is re-run automatically whenever conanfile changes
    // by making a file system property part of the probe's signature.
    property int _conanfileLastModified: conanfilePath ? File.lastModified(conanfilePath) : 0
    property path _projectBuildDirectory: project.buildDirectory

    // TODO: If minimal qbs version is 1.23 replace with FileInfo.executableSuffix()
    readonly property string executableSuffix: project.qbs.targetOS.contains("windows") ? ".exe" : ""

    configure: {
        if (executable === "conan")
            executable = executable + executableSuffix;

        if (conanfilePath && packageReference)
            throw("conanfilePath and packageReference must not be defined at the same time.");

        if (!conanfilePath && !packageReference)
            throw("Either conanfilePath or packageReference must be defined.");

        var reference = packageReference || FileInfo.cleanPath(conanfilePath);
        console.info("Probing '" + reference + "'. This might take a while...");
        if (conanfilePath && !File.exists(reference))
                throw("The conanfile '" + reference + "' does not exist.");

        var args = [
            "install", reference,
        ];

        if (options) {
            if (typeof options !== "object")
                throw("The property 'options' must be an object.");
            Object.keys(options).forEach(function(key,index) {
                args.push("-o");
                args.push(key + "=" + options[key]);
            });
        }

        if (settings) {
            if (typeof settings !== "object")
                throw("The property 'settings' must be an object.");
            Object.keys(settings).forEach(function(key,index) {
                args.push("-s");
                args.push(key + "=" + settings[key]);
            });
        }

        if (!generators.contains("json"))
            generators.push("json");

        for (var i = 0; i < generators.length; i++)
            args = args.concat(["-g", generators[i]]);

        for (var i = 0; i < additionalArguments.length; i++)
            args.push(additionalArguments[i]);

        for (var i = 0; i < profiles.length; i++) {
            args = args.concat(["-pr", profiles[i]]);
        }

        generatedFilesPath = FileInfo.cleanPath(_projectBuildDirectory +
                                                  "/genconan/" +
                                                  Utilities.getHash(args.join()));

        args = args.concat(["-if", generatedFilesPath]);

        if (remote) {
            args = args.concat(["--remote", remote]);
        }

        var p = new Process();
        console.info("Start conan: " + executable + " " + args.join(" "));
        p.exec(executable, args, true);

        if (generators.contains("json")) {
            if (!File.exists(generatedFilesPath + "/conanbuildinfo.json"))
                throw("No conanbuildinfo.json has been generated.");

            var jsonFile = new TextFile(generatedFilesPath + "/conanbuildinfo.json", TextFile.ReadOnly);
            json = JSON.parse(jsonFile.readAll());
            jsonFile.close();

            dependencies = {};
            for (var i = 0; i < json.dependencies.length; ++i) {
                var dep = json.dependencies[i];
                dependencies[dep.name] = dep;
            }
        }

        found = true;
    }
}
