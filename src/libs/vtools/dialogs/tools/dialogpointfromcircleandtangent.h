/************************************************************************
 **
 **  @file   dialogpointfromcircleandtangent.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef DIALOGPOINTFROMCIRCLEANDTANGENT_H
#define DIALOGPOINTFROMCIRCLEANDTANGENT_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogPointFromCircleAndTangent;
}

class DialogPointFromCircleAndTangent : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogPointFromCircleAndTangent(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogPointFromCircleAndTangent() override;

    auto GetPointName() const -> QString;
    void SetPointName(const QString &value);

    auto GetCircleCenterId() const -> quint32;
    void SetCircleCenterId(const quint32 &value);

    auto GetCircleRadius() const -> QString;
    void SetCircleRadius(const QString &value);

    auto GetTangentPointId() const -> quint32;
    void SetTangentPointId(quint32 value);

    auto GetCrossCirclesPoint() const -> CrossCirclesPoint;
    void SetCrossCirclesPoint(CrossCirclesPoint p);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

    void ShowDialog(bool click) override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    void PointChanged();

    void DeployCircleRadiusTextEdit();
    void FXCircleRadius();
    void EvalCircleRadius();

protected:
    void ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    void SaveData() override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto IsValid() const -> bool final;

private:
    Q_DISABLE_COPY_MOVE(DialogPointFromCircleAndTangent) // NOLINT

    Ui::DialogPointFromCircleAndTangent *ui;

    QTimer *m_timerCircleRadius{nullptr};
    QString m_circleRadius{};
    int m_formulaBaseHeightCircleRadius{0};
    QString m_pointName{};
    bool m_flagCircleRadius{false};
    bool m_flagName{true};
    bool m_flagError{true};
    /** @brief number number of handled objects */
    qint32 m_number{0};
    bool m_firstRelease{false};

    void FinishCreating();

    void InitIcons();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogPointFromCircleAndTangent::IsValid() const -> bool
{
    return m_flagCircleRadius && m_flagName && m_flagError;
}

#endif // DIALOGPOINTFROMCIRCLEANDTANGENT_H
