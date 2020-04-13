/************************************************************************
 **
 **  @file   vpiececarrousel.h
 **  @author Ronan Le Tiec
 **  @date   13 04, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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

#ifndef VPIECECARROUSEL_H
#define VPIECECARROUSEL_H

#include <QWidget>
#include <QComboBox>
#include <QScrollArea>

class VPieceCarrousel : public QWidget
{
    Q_OBJECT
public:
    explicit VPieceCarrousel(QWidget *parent = nullptr); 
    virtual ~VPieceCarrousel();

    void setOrientation(Qt::Orientation orientation);
signals:

public slots:

private:
    Q_DISABLE_COPY(VPieceCarrousel)
    QComboBox *comboBoxLayer;
    QScrollArea *mainScrollArea;
    QList<QWidget *> layers;

private slots:
    void ActiveLayerChanged(int index);
};

#endif // VPIECECARROUSEL_H
