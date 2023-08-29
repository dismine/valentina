/************************************************************************
 **
 **  @file   preferencesconfigurationpage.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 4, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#ifndef PREFERENCESCONFIGURATIONPAGE_H
#define PREFERENCESCONFIGURATIONPAGE_H

#include <QWidget>

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif

namespace Ui
{
class PreferencesConfigurationPage;
}

class PreferencesConfigurationPage : public QWidget
{
    Q_OBJECT // NOLINT

public:
    explicit PreferencesConfigurationPage(QWidget *parent = nullptr);
    ~PreferencesConfigurationPage() override;

    auto Apply() -> QStringList;

protected:
    void changeEvent(QEvent *event) override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(PreferencesConfigurationPage) // NOLINT
    Ui::PreferencesConfigurationPage *ui;
    bool m_langChanged{false};
    bool m_pieceLabelLangChanged{false};
    bool m_systemChanged{false};
    bool m_unitChanged{false};
    bool m_labelLangChanged{false};

    void SetLabelComboBox(const QStringList &list);
    void SetThemeModeComboBox();
    void SetPointerModeComboBox();
    void InitUnits();
    void RetranslateUi();
};

#endif // PREFERENCESCONFIGURATIONPAGE_H
