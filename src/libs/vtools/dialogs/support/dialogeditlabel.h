/************************************************************************
 **
 **  @file   dialogeditlabel.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 8, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2017 Valentina project
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

#ifndef DIALOGEDITLABEL_H
#define DIALOGEDITLABEL_H

#include <QDialog>
#include <QMap>

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif

namespace Ui
{
class DialogEditLabel;
}

struct VLabelTemplateLine;

class QMenu;
class VAbstractPattern;
class VPiece;
class VContainer;

class DialogEditLabel : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogEditLabel(const VAbstractPattern *doc, const VContainer *data, QWidget *parent = nullptr);
    ~DialogEditLabel() override;

    auto GetTemplate() const -> QVector<VLabelTemplateLine>;
    void SetTemplate(const QVector<VLabelTemplateLine> &lines);

    void SetPiece(const VPiece &piece);

private slots:
    void ShowLineDetails();
    void AddLine();
    void RemoveLine();
    void SaveLineText(const QString &text);
    void SaveFontStyle(bool checked);
    void SaveTextFormating(bool checked);
    void NewTemplate();
    void ExportTemplate();
    void ImportTemplate();
    void InsertPlaceholder();
    void TabChanged(int index);
    void SaveAdditionalFontSize(int i);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DialogEditLabel) // NOLINT
    Ui::DialogEditLabel *ui;
    QMenu *m_placeholdersMenu;
    const VAbstractPattern *m_doc;
    const VContainer *m_data;

    QMap<QString, QPair<QString, QString>> m_placeholders{};

    void SetupControls();
    auto SortedActions() const -> QMap<QString, QString>;
    void InitPlaceholdersMenu();
    void InitPlaceholders();

    auto ReplacePlaceholders(QString line) const -> QString;

    void InitPreviewLines(const QVector<VLabelTemplateLine> &lines);
};

#endif // DIALOGEDITLABEL_H
