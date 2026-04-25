/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Copyright (C) 2021 Ivan Komissarov (abbapoh@gmail.com)
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

function prepareSigntool(project, product, inputs, outputs, input, output) {
    var cmd, cmds = [];

    if (!product.codesign.enableCodeSigning)
        return cmds;

    var args = ["sign"];

    var subjectName = product.codesign.subjectName;
    if (subjectName)
        args.push("/n", subjectName);

    var rootSubjectName = product.codesign.rootSubjectName;
    if (rootSubjectName)
        args.push("/r", rootSubjectName);

    var hashAlgorithm = product.codesign.hashAlgorithm;
    if (hashAlgorithm)
        args.push("/fd", hashAlgorithm);

    var signingTimestamp = product.codesign.signingTimestamp;
    if (signingTimestamp)
        args.push("/tr", signingTimestamp);

    var timestampAlgorithm = product.codesign.timestampAlgorithm;
    if (timestampAlgorithm)
        args.push("/td", timestampAlgorithm);

    var certificatePath = product.codesign.certificatePath;
    if (certificatePath)
        args.push("/f", certificatePath);

    var certificatePassword = product.codesign.certificatePassword;
    if (certificatePassword)
        args.push("/p", certificatePassword);

    var crossCertificatePath = product.codesign.crossCertificatePath;
    if (crossCertificatePath)
        args.push("/ac", crossCertificatePath);

    args = args.concat(product.codesign.codesignFlags || []);

    var outputArtifact = outputs["codesign.signed_artifact"][0];
    args.push(outputArtifact.filePath);

    cmd = new Command(product.codesign.codesignPath, args);
    cmd.description = "signing " + outputArtifact.fileName;
    cmd.highlight = "linker";
    cmds.push(cmd);
    return cmds;
}
