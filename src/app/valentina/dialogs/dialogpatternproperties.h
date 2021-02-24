/************************************************************************
 **
 **  @file   dialogpatternproperties.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 2, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
 **  <https://gitlab.com/smart-pattern/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#ifndef DIALOGPATTERNPROPERTIES_H
#define DIALOGPATTERNPROPERTIES_H

#include <QDialog>
#include <QMap>

#include "../vmisc/def.h"
#include "../ifc/ifcdef.h"

class VPattern;
class VContainer;
class QCheckBox;

namespace Ui
{
    class DialogPatternProperties;
}

class DialogPatternProperties : public QDialog
{
    Q_OBJECT
public:
    explicit DialogPatternProperties(VPattern *doc, VContainer *pattern, QWidget *parent = nullptr);
    virtual ~DialogPatternProperties() override;
private slots:
    void Apply();
    void Ok();
    void DescEdited();
    void ChangeImage();
    void SaveImage();
private:
    Q_DISABLE_COPY(DialogPatternProperties)
    Ui::DialogPatternProperties *ui;
    VPattern               *doc;
    VContainer             *pattern;
    QMap<QCheckBox *, int> data;
    bool                   descriptionChanged;
    bool                   gradationChanged;
    bool                   defaultChanged;
    bool                   securityChanged;
    QAction                *deleteAction;
    QAction                *changeImageAction;
    QAction                *saveImageAction;
    QAction                *showImageAction;

    void         SaveDescription();
    void         SaveReadOnlyState();

    void         InitImage();
    QImage       GetImage();
};

#endif // DIALOGPATTERNPROPERTIES_H
