/************************************************************************
 **
 **  @file   dialogoperationtool.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 5, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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

#ifndef DIALOGOPERATIONTOOL_H
#define DIALOGOPERATIONTOOL_H

#include "dialogtool.h"
#include "../../tools/toolsdef.h"

class QAbstractButton;
class QGroupBox;
class QLabel;
class QPlainTextEdit;
class VCompleterLineEdit;
namespace VPE
{
class QtColorPicker;
}

struct OperationWidgets
{
    QListWidget *sourceList{nullptr};
    QLineEdit *name{nullptr};
    QLabel *labelName{nullptr};
    QLabel *labelStatus{nullptr};
    QLabel *labelGroupName{nullptr};
    QComboBox *penStyle{nullptr};
    VPE::QtColorPicker *color{nullptr};
    QComboBox *objectType{nullptr};
    QComboBox *newObject{nullptr};
    QAbstractButton *addObject{nullptr};
    QAbstractButton *removeObject{nullptr};
    QPlainTextEdit *notes{nullptr};
    VCompleterLineEdit *groupTags{nullptr};
    QLineEdit *visibilityGroupLine{nullptr};
    QGroupBox *visibilityGroup{nullptr};
};

class DialogOperationTool : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    auto GetVisibilityGroupName() const -> QString;
    void SetVisibilityGroupName(const QString &name) const;

    auto HasLinkedVisibilityGroup() const -> bool;
    void SetHasLinkedVisibilityGroup(bool linked) const;

    void SetVisibilityGroupTags(const QStringList &tags) const;
    auto GetVisibilityGroupTags() const -> QStringList;

    void SetNotes(const QString &notes) const;
    auto GetNotes() const -> QString;

    void SetGroupCategories(const QStringList &categories) override;

    auto GetSourceObjects() const -> QVector<SourceItem>;
    void SetSourceObjects(const QVector<SourceItem> &value);

    void SetDestinationObjects(const QVector<DestinationItem> &value);

    void CheckDependencyTreeComplete() override;

public slots:
    void ClearSourceObjects() override;
    void GroupNameChanged();
    void ShowSourceDetails(int row);
    void CurrentObjectChanged(int row) const;
    void NameChanged(const QString &text);
    void PenStyleChanged() const;
    void ObjectTypeChanged(int index) const;
    void NewObjectChanged() const;
    void AddNewObject();
    void ColorChanged() const;
    void BulkRename();
    void RemoveObject();

protected:
    explicit DialogOperationTool(const VContainer *data, VAbstractPattern *doc, quint32 toolId,
                                 QWidget *parent = nullptr);

    virtual auto Widgets() const -> OperationWidgets = 0;

    virtual void OnSourceObjectsSet() {}

    void FillSourceList();
    auto SaveSourceObjects() const -> QVector<SourceItem>;

    auto SourceObjects() -> QVector<SourceItem> & { return m_sourceObjects; }
    auto IsStage1() const -> bool { return stage1; }
    void SetStage1(bool value) { stage1 = value; }
    auto IsFlagName() const -> bool { return flagName; }
    auto IsFlagGroupName() const -> bool { return flagGroupName; }
    auto IsFlagError() const -> bool { return flagError; }
    void SetFlagError(bool value) { flagError = value; }
    auto GroupTags() -> QStringList & { return m_groupTags; }

private:
    QVector<SourceItem> m_sourceObjects{};
    QVector<DestinationItem> m_destination{};

    bool stage1{true};
    bool flagName{true};
    bool flagGroupName{true};
    bool flagError{false};
    QStringList m_groupTags{};
    bool m_dependencyReady{true};
};

#endif // DIALOGOPERATIONTOOL_H
