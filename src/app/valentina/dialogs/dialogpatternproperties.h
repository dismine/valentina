/************************************************************************
 **
 **  @file   dialogpatternproperties.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 2, 2014
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

#ifndef DIALOGPATTERNPROPERTIES_H
#define DIALOGPATTERNPROPERTIES_H

#include <QDialog>
#include <QMap>
#include <QPointer>

class VPattern;
class VContainer;
class QCheckBox;
class QCompleter;
class QTemporaryFile;

namespace Ui
{
class DialogPatternProperties;
}

class DialogPatternProperties : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogPatternProperties(VPattern *doc, VContainer *pattern, QWidget *parent = nullptr);
    ~DialogPatternProperties() override;
signals:
    void UpddatePieces();

protected:
    auto eventFilter(QObject *object, QEvent *event) -> bool override;
private slots:
    void Apply();
    void Ok();
    void DescEdited();
    void ChangeImage();
    void SaveImage();
    void ShowImage();
    void BrowseLabelPath();
    void LabelPathChanged(const QString &text);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DialogPatternProperties) // NOLINT
    Ui::DialogPatternProperties *ui;
    VPattern *m_doc;
    VContainer *m_pattern;
    QMap<QCheckBox *, int> m_data{};
    bool m_descriptionChanged{false};
    bool m_defaultChanged{false};
    bool m_securityChanged{false};
    QAction *m_deleteAction{nullptr};
    QAction *m_changeImageAction{nullptr};
    QAction *m_saveImageAction{nullptr};
    QAction *m_showImageAction{nullptr};
    QCompleter *m_completerLength{nullptr};
    QCompleter *m_completerWidth{nullptr};
    QStringList m_variables{};
    QString m_oldPassmarkLength{};
    QString m_oldPassmarkWidth{};
    QPointer<QTemporaryFile> m_tmpImage{};

    void SaveDescription();
    void SaveReadOnlyState();

    void InitImage();

    void ValidatePassmarkLength() const;
    void ValidatePassmarkWidth() const;
};

#endif // DIALOGPATTERNPROPERTIES_H
