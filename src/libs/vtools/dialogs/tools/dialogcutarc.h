/************************************************************************
 **
 **  @file   dialogcutarc.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 1, 2014
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

#ifndef DIALOGCUTARC_H
#define DIALOGCUTARC_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogCutArc;
}

/**
 * @brief The DialogCutArc class dialog for ToolCutArc.
 */
class DialogCutArc final : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogCutArc(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogCutArc() override;

    auto GetPointName() const -> QString;
    void SetPointName(const QString &value);

    auto GetFormula() const -> QString;
    void SetFormula(const QString &value);

    auto getArcId() const -> quint32;
    void setArcId(quint32 value);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

    void SetAliasSuffix1(const QString &alias);
    auto GetAliasSuffix1() const -> QString;

    void SetAliasSuffix2(const QString &alias);
    auto GetAliasSuffix2() const -> QString;

    void ShowDialog(bool click) override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    /**
     * @brief DeployFormulaTextEdit grow or shrink formula input
     */
    void DeployFormulaTextEdit();

    void FXLength();
    void EvalFormula();

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
    void ArcChanged();
    void ValidateAlias();

private:
    Q_DISABLE_COPY_MOVE(DialogCutArc) // NOLINT
    /** @brief ui keeps information about user interface */
    Ui::DialogCutArc *ui;

    /** @brief formula string with formula */
    QString m_formula{};
    QString m_pointName{};

    /** @brief formulaBaseHeight base height defined by dialogui */
    int m_formulaBaseHeight{0};

    QTimer *m_timerFormula;

    bool m_flagFormula{false};
    bool m_flagName{true};
    bool m_flagAlias1{true};
    bool m_flagAlias2{true};

    QString m_originAliasSuffix1{};
    QString m_originAliasSuffix2{};

    bool m_firstRelease{false};

    void FinishCreating();

    void InitIcons();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogCutArc::IsValid() const -> bool
{
    return m_flagFormula && m_flagName && m_flagAlias1 && m_flagAlias2;
}

#endif // DIALOGCUTARC_H
