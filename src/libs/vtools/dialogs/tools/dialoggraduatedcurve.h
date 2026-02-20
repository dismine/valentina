/************************************************************************
 **
 **  @file   dialoggraduatedcurve.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   4 12, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#ifndef DIALOGGRADUATEDCURVE_H
#define DIALOGGRADUATEDCURVE_H

#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "dialogtool.h"

namespace Ui
{
class DialogGraduatedCurve;
}

struct VRawGraduatedCurveOffset;

struct VGraduatedCurveOffsetFormula
{
    QString name{};
    VFormula formula{};
    QSharedPointer<VContainer> formulaData{};
    QString description{};
};

class DialogGraduatedCurve : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogGraduatedCurve(const VContainer *data,
                                  VAbstractPattern *doc,
                                  quint32 toolId,
                                  QWidget *parent = nullptr);
    ~DialogGraduatedCurve() override;

    auto GetOriginCurveId() const -> quint32;
    void SetOriginCurveId(const quint32 &value);

    auto GetOffsets() const -> QVector<VRawGraduatedCurveOffset>;
    void SetOffsets(const QVector<VRawGraduatedCurveOffset> &offsets);

    auto GetPenStyle() const -> QString;
    void SetPenStyle(const QString &value);

    auto GetColor() const -> QString;
    void SetColor(const QString &value);

    auto GetApproximationScale() const -> qreal;
    void SetApproximationScale(qreal value);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

    void SetName(const QString &name);
    auto GetName() const -> QString;

    void SetAliasSuffix(const QString &alias);
    auto GetAliasSuffix() const -> QString;

    void ShowDialog(bool click) override;

    void CheckDependencyTreeComplete() override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;

protected:
    void ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    void SaveData() override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto IsValid() const -> bool override;

private slots:
    void ValidateName();
    void ValidateAlias();
    void ShowOffsetDetails();
    void AddOffset();
    void RemoveOffset();
    void MoveUp();
    void MoveDown();
    void DeployFormula();
    void Fx();
    void SaveOffsetName(const QString &text);
    void SaveOffsetDescription();
    void SaveOffsetFormula();

private:
    Q_DISABLE_COPY_MOVE(DialogGraduatedCurve) // NOLINT
    Ui::DialogGraduatedCurve *ui;

    bool m_flagSuffix{true};
    bool m_flagAlias{true};

    QString m_originName{};
    QString m_originAliasSuffix{};

    bool m_firstRelease{false};

    int m_formulaBaseHeight{0};

    QVector<VGraduatedCurveOffsetFormula> m_offsets{};

    void FinishCreating();

    void InitIcons();

    void FillOffsets();

    void Controls();
    void EnableDetails(bool enabled);

    auto GetOffsetName(bool translate) const -> QString;

    auto VisualizationOffsets() const -> QVector<VRawGraduatedCurveOffset>;

    void ShowHeaderUnits(int column);
};

#endif // DIALOGGRADUATEDCURVE_H
