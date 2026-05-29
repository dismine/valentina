/************************************************************************
 **
 **  @file   dialogrotation.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 4, 2016
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

#ifndef DIALOGROTATION_H
#define DIALOGROTATION_H

#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "dialogoperationtool.h"

namespace Ui
{
class DialogRotation;
}

class DialogRotation final : public DialogOperationTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogRotation(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogRotation() override;

    auto GetOrigPointId() const -> quint32;
    void SetOrigPointId(quint32 value);

    auto GetAngle() const -> QString;
    void SetAngle(const QString &value);

    void ShowDialog(bool click) override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;

private slots:
    /** @brief DeployAngleTextEdit grow or shrink formula input */
    void DeployAngleTextEdit();
    void FXAngle();
    void EvalAngle();

protected:
    void ShowVisualization() override;

    /** @brief SaveData Put dialog data in local variables */
    void SaveData() override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto IsValid() const -> bool override;

    void OnSourceObjectsSet() override;

    auto SourceListWidget() const -> QListWidget * override;
    auto NameLineEdit() const -> QLineEdit * override;
    auto LabelName() const -> QLabel * override;
    auto LabelStatus() const -> QLabel * override;
    auto LabelGroupName() const -> QLabel * override;
    auto PenStyleComboBox() const -> QComboBox * override;
    auto ColorButton() const -> VPE::QtColorPicker * override;
    auto ObjectTypeComboBox() const -> QComboBox * override;
    auto NewObjectComboBox() const -> QComboBox * override;
    auto AddObjectButton() const -> QAbstractButton * override;
    auto RemoveObjectButton() const -> QAbstractButton * override;
    auto NotesPlainTextEdit() const -> QPlainTextEdit * override;
    auto GroupTagsLineEdit() const -> VCompleterLineEdit * override;
    auto VisibilityGroupLineEdit() const -> QLineEdit * override;
    auto VisibilityGroupBox() const -> QGroupBox * override;

private slots:
    void PointChanged();

private:
    Q_DISABLE_COPY_MOVE(DialogRotation) // NOLINT
    Ui::DialogRotation *ui;

    /** @brief timerAngle timer of check formula of angle */
    QTimer *timerAngle;

    /** @brief angle formula of angle */
    QString formulaAngle{};

    /** @brief formulaBaseHeightAngle base height defined by dialogui */
    int formulaBaseHeightAngle{0};

    bool m_firstRelease{false};

    /** @brief flagAngle true if value of angle is correct */
    bool flagAngle{false};

    void InitIcons();
};

#endif // DIALOGROTATION_H
