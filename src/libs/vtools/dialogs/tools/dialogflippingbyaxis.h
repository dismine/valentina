/************************************************************************
 **
 **  @file   dialogflippingbyaxis.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 9, 2016
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

#ifndef DIALOGFLIPPINGBYAXIS_H
#define DIALOGFLIPPINGBYAXIS_H

#include "dialogtool.h"

#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../../tools/toolsdef.h"
#include "../vmisc/def.h"

namespace Ui
{
class DialogFlippingByAxis;
}

class DialogFlippingByAxis : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogFlippingByAxis(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogFlippingByAxis() override;

    auto GetOriginPointId() const -> quint32;
    void SetOriginPointId(quint32 value);

    auto GetAxisType() const -> AxisType;
    void SetAxisType(AxisType type);

    auto GetSuffix() const -> QString;
    void SetSuffix(const QString &value);

    auto GetVisibilityGroupName() const -> QString;
    void SetVisibilityGroupName(const QString &name);

    auto HasLinkedVisibilityGroup() const -> bool;
    void SetHasLinkedVisibilityGroup(bool linked);

    void SetVisibilityGroupTags(const QStringList &tags);
    auto GetVisibilityGroupTags() const -> QStringList;

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

    void SetGroupCategories(const QStringList &categories) override;

    void ShowDialog(bool click) override;

    auto GetSourceObjects() const -> QVector<SourceItem>;
    void SetSourceObjects(const QVector<SourceItem> &value);

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    void SelectedObject(bool selected, quint32 object, quint32 tool) override;

private slots:
    void SuffixChanged();
    void GroupNameChanged();
    void ShowSourceDetails(int row);
    void AliasChanged(const QString &text);
    void PenStyleChanged();
    void ColorChanged();

protected:
    void ShowVisualization() override;

    /** @brief SaveData Put dialog data in local variables */
    void SaveData() override;
    auto IsValid() const -> bool final;

private slots:
    void PointChanged();

private:
    Q_DISABLE_COPY_MOVE(DialogFlippingByAxis) // NOLINT

    Ui::DialogFlippingByAxis *ui;

    QVector<SourceItem> sourceObjects{};

    bool stage1;

    QString m_suffix;

    bool flagName;
    bool flagGroupName;
    bool flagError;
    bool flagAlias{true};

    QStringList m_groupTags{};

    static void FillComboBoxAxisType(QComboBox *box);

    void FillSourceList();

    void ValidateSourceAliases();
    void SetAliasValid(quint32 id, bool valid);
};

#endif // DIALOGFLIPPINGBYAXIS_H
