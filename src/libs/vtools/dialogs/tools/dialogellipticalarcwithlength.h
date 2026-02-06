/************************************************************************
 **
 **  @file   dialogellipticalarcwithlength.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 10, 2025
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
#ifndef DIALOGELLIPTICALARCWITHLENGTH_H
#define DIALOGELLIPTICALARCWITHLENGTH_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogEllipticalArcWithLength;
}

class DialogEllipticalArcWithLength final : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogEllipticalArcWithLength(const VContainer *data,
                                           VAbstractPattern *doc,
                                           quint32 toolId,
                                           QWidget *parent = nullptr);
    ~DialogEllipticalArcWithLength() override;

    auto GetCenter() const -> quint32;
    void SetCenter(const quint32 &value);

    auto GetRadius1() const -> QString;
    void SetRadius1(const QString &value);

    auto GetRadius2() const -> QString;
    void SetRadius2(const QString &value);

    auto GetF1() const -> QString;
    void SetF1(const QString &value);

    auto GetLength() const -> QString;
    void SetLength(const QString &value);

    auto GetRotationAngle() const -> QString;
    void SetRotationAngle(const QString &value);

    auto GetPenStyle() const -> QString;
    void SetPenStyle(const QString &value);

    auto GetColor() const -> QString;
    void SetColor(const QString &value);

    auto GetApproximationScale() const -> qreal;
    void SetApproximationScale(qreal value);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

    void SetAliasSuffix(const QString &alias);
    auto GetAliasSuffix() const -> QString;

    void ShowDialog(bool click) override;

    void CheckDependencyTreeComplete() override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;

    void DeployRadius1TextEdit();
    void DeployRadius2TextEdit();
    void DeployF1TextEdit();
    void DeployLengthTextEdit();
    void DeployRotationAngleTextEdit();

    void FXRadius1();
    void FXRadius2();
    void FXF1();
    void FXLength();
    void FXRotationAngle();

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
    void ValidateAlias();

private:
    Q_DISABLE_COPY_MOVE(DialogEllipticalArcWithLength) // NOLINT
    Ui::DialogEllipticalArcWithLength *ui;

    /** @brief flagRadius1 true if value of radius1 is correct */
    bool m_flagRadius1{false};

    /** @brief flagRadius2 true if value of radius2 is correct */
    bool m_flagRadius2{false};

    /** @brief flagF1 true if value of first angle is correct */
    bool m_flagF1{false};

    bool m_flagLength{false};

    /** @brief flagRotationAngle true if value of rotation angle is correct */
    bool m_flagRotationAngle{false};

    bool m_flagAlias{true};

    /** @brief timerRadius1 timer of check formula of radius1 */
    QTimer *m_timerRadius1;

    /** @brief timerRadius2 timer of check formula of radius2 */
    QTimer *m_timerRadius2;

    /** @brief timerF1 timer of check formula of first angle */
    QTimer *m_timerF1;

    QTimer *m_timerLength;

    /** @brief timerRotationAngle timer of check formula of rotation angle */
    QTimer *m_timerRotationAngle;

    /** @brief radius1 formula of radius1 */
    QString m_radius1{};

    /** @brief radius2 formula of radius2 */
    QString m_radius2{};

    /** @brief f1 formula of first angle */
    QString m_f1{};

    QString m_length{};

    /** @brief rotationAngle formula of rotation angle */
    QString m_rotationAngle{};

    int m_formulaBaseHeightRadius1{0};
    int m_formulaBaseHeightRadius2{0};
    int m_formulaBaseHeightF1{0};
    int m_formulaBaseHeightLength{0};
    int m_formulaBaseHeightRotationAngle{0};

    QString m_originAliasSuffix{};

    bool m_firstRelease{false};

    int m_stage{0};

    void Radius1();
    void Radius2();
    void Length();
    void EvalF();
    void RotationAngle();

    void FinishCreating();

    void InitIcons();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogEllipticalArcWithLength::IsValid() const -> bool
{
    return m_flagRadius1 && m_flagRadius2 && m_flagF1 && m_flagLength && m_flagAlias && m_flagRotationAngle;
}

#endif // DIALOGELLIPTICALARCWITHLENGTH_H
