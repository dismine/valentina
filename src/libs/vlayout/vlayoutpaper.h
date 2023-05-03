/************************************************************************
 **
 **  @file   vlayoutpaper.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 1, 2015
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

#ifndef VLAYOUTPAPER_H
#define VLAYOUTPAPER_H

#include <qcompilerdetection.h>
#include <QSharedDataPointer>
#include <QTypeInfo>
#include <QtGlobal>
#include <atomic>
#include <QGraphicsPathItem>
#include <QtCore/qcontainerfwd.h>

#include "../vmisc/defglobal.h"

class VBestSquare;
class VLayoutPaperData;
class VLayoutPiece;
class QGraphicsRectItem;
class QRectF;
class QGraphicsItem;
class QMutex;

class VLayoutPaper
{
public:
    VLayoutPaper();
    VLayoutPaper(int height, int width, qreal layoutWidth);
    VLayoutPaper(const VLayoutPaper &paper);

    ~VLayoutPaper();

    auto operator=(const VLayoutPaper &paper) -> VLayoutPaper &;
#ifdef Q_COMPILER_RVALUE_REFS
    VLayoutPaper(VLayoutPaper &&paper) Q_DECL_NOTHROW;
    auto operator=(VLayoutPaper &&paper) Q_DECL_NOTHROW->VLayoutPaper &;
#endif

    auto GetHeight() const -> int;
    void SetHeight(int height);

    auto GetWidth() const -> int;
    void SetWidth(int width);

    auto GetLayoutWidth() const -> qreal;
    void  SetLayoutWidth(qreal width);

    auto GetShift() const -> qreal;
    void    SetShift(qreal shift);

    auto GetRotate() const -> bool;
    void SetRotate(bool value);

    auto GetFollowGrainline() const -> bool;
    void SetFollowGrainline(bool value);

    auto GetRotationNumber() const -> int;
    void SetRotationNumber(int value);

    auto IsSaveLength() const -> bool;
    void SetSaveLength(bool value);

    void SetPaperIndex(quint32 index);

    auto IsOriginPaperPortrait() const -> bool;
    void SetOriginPaperPortrait(bool portrait);

    auto ArrangeDetail(const VLayoutPiece &detail, std::atomic_bool &stop) -> bool;
    auto Count() const -> vsizetype;
    Q_REQUIRED_RESULT auto GetPaperItem(bool autoCropLength, bool autoCropWidth, bool textAsPaths) const
        -> QGraphicsRectItem *;
    Q_REQUIRED_RESULT auto GetGlobalContour() const -> QGraphicsPathItem *;
    Q_REQUIRED_RESULT auto GetItemDetails(bool textAsPaths) const -> QList<QGraphicsItem *>;

    auto GetDetails() const -> QVector<VLayoutPiece>;
    void                  SetDetails(const QVector<VLayoutPiece>& details);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void                  SetDetails(const QList<VLayoutPiece>& details);
#endif

    auto DetailsBoundingRect() const -> QRectF;

    auto Efficiency() const -> qreal;

private:
    QSharedDataPointer<VLayoutPaperData> d;

    auto SaveResult(const VBestSquare &bestResult, const VLayoutPiece &detail
#ifdef LAYOUT_DEBUG
                    ,
                    QMutex *mutex
#endif
                    ) -> bool;
};

Q_DECLARE_TYPEINFO(VLayoutPaper, Q_MOVABLE_TYPE); // NOLINT

#endif // VLAYOUTPAPER_H
