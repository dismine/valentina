/************************************************************************
 **
 **  @file   dialogcubicbezierpath.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 3, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef DIALOGCUBICBEZIERPATH_H
#define DIALOGCUBICBEZIERPATH_H

#include <QMetaObject>
#include <QObject>
#include <QSet>
#include <QString>
#include <QtGlobal>

#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogCubicBezierPath;
}

class DialogCubicBezierPath : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogCubicBezierPath(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogCubicBezierPath() override;

    auto GetPath() const -> VCubicBezierPath;
    void SetPath(const VCubicBezierPath &value);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    void ShowDialog(bool click) override;

protected:
    void ShowVisualization() override;
    void SaveData() override;
    auto IsValid() const -> bool final;

private slots:
    void PointChanged(int row);
    void currentPointChanged(int index);
    void ValidateAlias();

private:
    Q_DISABLE_COPY_MOVE(DialogCubicBezierPath) // NOLINT
    Ui::DialogCubicBezierPath *ui;

    /** @brief path cubic bezier path */
    VCubicBezierPath path;

    qint32 newDuplicate;

    bool flagError;
    bool flagAlias{true};

    QString originAliasSuffix{};

    void NewItem(const VPointF &point);
    void DataPoint(const VPointF &p);
    void SavePath();
    auto AllPathBackboneIds() const -> QSet<quint32>;
    auto IsPathValid() const -> bool;
    auto ExtractPath() const -> VCubicBezierPath;
    void ValidatePath();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogCubicBezierPath::IsValid() const -> bool
{
    return flagError && flagAlias;
}

#endif // DIALOGCUBICBEZIERPATH_H
