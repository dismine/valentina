/************************************************************************
 **
 **  @file   dialogellipticalarc.h
 **  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
 **  @date   15 9, 2016
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

#ifndef DIALOGELLIPTICALARC_H
#define DIALOGELLIPTICALARC_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogEllipticalArc;
}

class DialogEllipticalArc : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogEllipticalArc(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogEllipticalArc() override;

    auto GetCenter() const -> quint32;
    void SetCenter(quint32 value);

    auto GetRadius1() const -> QString;
    void SetRadius1(const QString &value);

    auto GetRadius2() const -> QString;
    void SetRadius2(const QString &value);

    auto GetF1() const -> QString;
    void SetF1(const QString &value);

    auto GetF2() const -> QString;
    void SetF2(const QString &value);

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

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    /**
     * @brief DeployFormulaTextEdit grow or shrink formula input
     */
    void DeployRadius1TextEdit();
    void DeployRadius2TextEdit();
    void DeployF1TextEdit();
    void DeployF2TextEdit();
    void DeployRotationAngleTextEdit();

    void FXRadius1();
    void FXRadius2();
    void FXF1();
    void FXF2();
    void FXRotationAngle();

protected:
    void ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    void SaveData() override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto IsValid() const -> bool final;

private slots:
    void ValidateAlias();

private:
    Q_DISABLE_COPY_MOVE(DialogEllipticalArc) // NOLINT

    /** @brief ui keeps information about user interface */
    Ui::DialogEllipticalArc *ui;

    /** @brief flagRadius1 true if value of radius1 is correct */
    bool m_flagRadius1{false};

    /** @brief flagRadius2 true if value of radius2 is correct */
    bool m_flagRadius2{false};

    /** @brief flagF1 true if value of first angle is correct */
    bool m_flagF1{false};

    /** @brief flagF2 true if value of second angle is correct */
    bool m_flagF2{false};

    /** @brief flagRotationAngle true if value of rotation angle is correct */
    bool m_flagRotationAngle{false};

    bool m_flagAlias{true};

    /** @brief timerRadius1 timer of check formula of radius1 */
    QTimer *m_timerRadius1;

    /** @brief timerRadius2 timer of check formula of radius2 */
    QTimer *m_timerRadius2;

    /** @brief timerF1 timer of check formula of first angle */
    QTimer *m_timerF1;

    /** @brief timerF2 timer of check formula of second angle */
    QTimer *m_timerF2;

    /** @brief timerRotationAngle timer of check formula of rotation angle */
    QTimer *m_timerRotationAngle;

    /** @brief radius1 formula of radius1 */
    QString m_radius1{};

    /** @brief radius2 formula of radius2 */
    QString m_radius2{};

    /** @brief f1 formula of first angle */
    QString m_f1{};

    /** @brief f2 formula of second angle */
    QString m_f2{};

    /** @brief rotationAngle formula of rotation angle */
    QString m_rotationAngle{};

    /** @brief formulaBaseHeight base height defined by dialogui */
    int m_formulaBaseHeightRadius1{0};
    int m_formulaBaseHeightRadius2{0};
    int m_formulaBaseHeightF1{0};
    int m_formulaBaseHeightF2{0};
    int m_formulaBaseHeightRotationAngle{0};

    qreal m_angleF1{INT_MIN};
    qreal m_angleF2{INT_MIN};
    qreal m_angleRotation{INT_MIN};

    QString m_originAliasSuffix{};

    bool m_firstRelease{false};

    int m_stage{0};

    void EvalRadiuses();
    void EvalAngles();

    void FinishCreating();

    void InitIcons();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogEllipticalArc::IsValid() const -> bool
{
    return m_flagRadius1 && m_flagRadius2 && m_flagF1 && m_flagF2 && m_flagRotationAngle && m_flagAlias;
}

#endif // DIALOGELLIPTICALARC_H
