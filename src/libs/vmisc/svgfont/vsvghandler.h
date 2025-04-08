/************************************************************************
 **
 **  @file   vsvghandler.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 4, 2025
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
#ifndef VSVGHANDLER_H
#define VSVGHANDLER_H

#include <QPainterPath>
#include <QStack>
#include <QXmlStreamReader>

class VSvgHandler
{
public:
    explicit VSvgHandler(const QString &filePath);

    void Render(QPainter *painter);

    auto PageSize() const -> QRectF;

private:
    QXmlStreamReader m_xml;
    QPainterPath m_path;
    QTransform m_currentTransform;
    QStack<QTransform> m_transformStack;
    QStack<QPainterPath> m_pathStack;
    QRectF m_pageSize;

    void ToPainterPath(const QString &filePath);

    void ResetState();
    void HandleStartElement();
    void ParseSvgAttributes();
    void InitialTransformFromViewBox();
    void SaveCurrentGroup();
    void RestorePreviousGroup();
    void ParsePathElement();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VSvgHandler::PageSize() const -> QRectF
{
    return m_pageSize;
}

#endif // VSVGHANDLER_H
