/************************************************************************
 **
 **  @file   dialogcubicbezier.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 3, 2016
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

#ifndef DIALOGCUBICBEZIER_H
#define DIALOGCUBICBEZIER_H

#include <QMetaObject>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QtGlobal>

#include "../vgeometry/vcubicbezier.h"
#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogCubicBezier;
}

class DialogCubicBezier : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogCubicBezier(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogCubicBezier() override;

    auto GetSpline() const -> VCubicBezier;
    void SetSpline(const VCubicBezier &spline);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    void PointNameChanged() override;

protected:
    void ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    void SaveData() override;
    auto IsValid() const -> bool final;

private slots:
    void ValidateAlias();

private:
    Q_DISABLE_COPY_MOVE(DialogCubicBezier) // NOLINT
    Ui::DialogCubicBezier *ui;

    /** @brief spl spline */
    VCubicBezier spl;

    qint32 newDuplicate;

    bool flagError;
    bool flagAlias{true};

    QString originAliasSuffix{};

    /** @brief number number of handled objects */
    qint32 number{0};

    auto GetP1() const -> const QSharedPointer<VPointF>;
    auto GetP2() const -> const QSharedPointer<VPointF>;
    auto GetP3() const -> const QSharedPointer<VPointF>;
    auto GetP4() const -> const QSharedPointer<VPointF>;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogCubicBezier::IsValid() const -> bool
{
    return flagError && flagAlias;
}

#endif // DIALOGCUBICBEZIER_H
