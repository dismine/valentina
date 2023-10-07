/************************************************************************
 **
 **  @file   dialogarc.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#ifndef DIALOGARC_H
#define DIALOGARC_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogArc;
}

/**
 * @brief The DialogArc class dialog for ToolArc. Help create arc and edit option.
 */
class DialogArc final : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogArc(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogArc() override;

    auto GetCenter() const -> quint32;
    void SetCenter(const quint32 &value);

    auto GetRadius() const -> QString;
    void SetRadius(const QString &value);

    auto GetF1() const -> QString;
    void SetF1(const QString &value);

    auto GetF2() const -> QString;
    void SetF2(const QString &value);

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
    void DeployFormulaTextEdit();
    void DeployF1TextEdit();
    void DeployF2TextEdit();

    void FXRadius();
    void FXF1();
    void FXF2();

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
    Q_DISABLE_COPY_MOVE(DialogArc) // NOLINT

    /** @brief ui keeps information about user interface */
    Ui::DialogArc *ui;

    /** @brief flagRadius true if value of radius is correct */
    bool m_flagRadius{false};

    /** @brief flagF1 true if value of first angle is correct */
    bool m_flagF1{false};

    /** @brief flagF2 true if value of second angle is correct */
    bool m_flagF2{false};

    bool m_flagAlias{true};

    /** @brief timerRadius timer of check formula of radius */
    QTimer *m_timerRadius;

    /** @brief timerF1 timer of check formula of first angle */
    QTimer *m_timerF1;

    /** @brief timerF2 timer of check formula of second angle */
    QTimer *m_timerF2;

    /** @brief radius formula of radius */
    QString m_radius{};

    /** @brief f1 formula of first angle */
    QString m_f1{};

    /** @brief f2 formula of second angle */
    QString m_f2{};

    /** @brief formulaBaseHeight base height defined by dialogui */
    int m_formulaBaseHeight{0};
    int m_formulaBaseHeightF1{0};
    int m_formulaBaseHeightF2{0};

    qreal m_angleF1{INT_MIN};
    qreal m_angleF2{INT_MIN};

    QString m_originAliasSuffix{};

    bool m_stageRadius{true};
    bool m_stageF1{false};

    bool m_firstRelease{false};

    void EvalRadius();
    void EvalF();

    void FinishCreating();

    void InitIcons();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogArc::IsValid() const -> bool
{
    return m_flagRadius && m_flagF1 && m_flagF2 && m_flagAlias;
}

#endif // DIALOGARC_H
