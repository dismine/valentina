/************************************************************************
 **
 **  @file   vwidgetdependencies.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 12, 2025
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
#ifndef VWIDGETDEPENDENCIES_H
#define VWIDGETDEPENDENCIES_H

#include "../vmisc/typedef.h"

#include <QObject>
#include <QWidget>

class VAbstractPattern;
class QGraphicsItem;
class VDependencyTreeModel;
class VTreeStateManager;
class VDependencyFilterProxyModel;

namespace Ui
{
class VWidgetDependencies;
}

class VWidgetDependencies : public QWidget
{
    Q_OBJECT // NOLINT

public:
    explicit VWidgetDependencies(VAbstractPattern *doc, QWidget *parent = nullptr);
    ~VWidgetDependencies() override;

public slots:
    void UpdateDependencies();
    void ShowDependency(QGraphicsItem *item);

signals:
    void Highlight(quint32 id);

private:
    Q_DISABLE_COPY_MOVE(VWidgetDependencies)
    Ui::VWidgetDependencies *ui;
    VAbstractPattern *m_doc;
    VDependencyTreeModel *m_model;
    VDependencyFilterProxyModel *m_proxyModel;
    VTreeStateManager *m_stateManager{nullptr};
    int m_indexPatternBlock{-1};

    auto RootTools() const -> QVector<vidtype>;
};

#endif // VWIDGETDEPENDENCIES_H
