/************************************************************************
 **
 **  @file   vhpglengine.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 7, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef VHPGLENGINE_H
#define VHPGLENGINE_H

#include <QPoint>
#include <QSize>
#include <QString>
#include <QtMath>

// Header <ciso646> is removed in C++20.
#if __cplusplus <= 201703L
#include <ciso646> // and, not, or
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif

class VLayoutPiece;
class VLayoutPoint;
class QTextStream;
class VTextManager;
class QPainterPath;

class VHPGLEngine
{
    friend class VHPGLPaintDevice;

public:
    VHPGLEngine();
    ~VHPGLEngine() = default;

    auto isActive() const -> bool;
    void setActive(bool newState);

    auto GetSize() const -> QSize;
    void SetSize(QSize size);

    auto GetFileName() const -> QString;
    void SetFileName(const QString &filename);

    auto GetSingleLineFont() const -> bool;
    void SetSingleLineFont(bool singleLineFont);

    auto GetSingleStrokeOutlineFont() const -> bool;
    void SetSingleStrokeOutlineFont(bool singleStrokeOutlineFont);

    auto GetPenWidth() const -> int;
    void SetPenWidth(int newPenWidth);

    auto GetXScale() const -> qreal;
    void SetXScale(const qreal &xscale);

    auto GetYScale() const -> qreal;
    void SetYScale(const qreal &yscale);

    void SetBoundaryTogetherWithNotches(bool value);
    auto IsBoundaryTogetherWithNotches() const -> bool;

    void SetIsertNewLine(bool insert);

    static auto SortDetails(const QVector<VLayoutPiece> &details) -> QList<VLayoutPiece>;

    auto GetShowGrainline() const -> bool;
    void SetShowGrainline(bool newShowGrainline);

private:
    Q_DISABLE_COPY_MOVE(VHPGLEngine) // NOLINT
    bool m_ver2{false};
    bool m_active{false};
    QSize m_size{};
    QString m_fileName{};
    bool m_inserNewLine{true};
    QPoint m_currentPos{-1, -1};
    bool m_singleLineFont{false};
    bool m_singleStrokeOutlineFont{false};
    int m_penWidthPx;
    qreal m_xscale{1};
    qreal m_yscale{1};
    bool m_showGrainline{true};
    bool m_togetherWithNotches{false};

    auto GenerateHPGL(const QVector<VLayoutPiece> &details) -> bool;
    auto GenerateHPGL2(const QVector<VLayoutPiece> &details) -> bool;

    void HPComand(QTextStream &out, const QString &mnemonic, const QString &parameters = QString()) const;
    void GenerateHPGLHeader(QTextStream &out);
    void ExportDetails(QTextStream &out, const QList<VLayoutPiece> &details);
    void GenerateHPGLFooter(QTextStream &out);

    void PlotSewLine(QTextStream &out, const VLayoutPiece &detail);
    void PlotSeamAllowance(QTextStream &out, const VLayoutPiece &detail);
    void PlotInternalPaths(QTextStream &out, const VLayoutPiece &detail);
    void PlotPlaceLabels(QTextStream &out, const VLayoutPiece &detail);
    void PlotPassmarks(QTextStream &out, const VLayoutPiece &detail);
    void PlotLabels(QTextStream &out, const VLayoutPiece &detail);
    void PlotGrainline(QTextStream &out, const VLayoutPiece &detail);

    void PlotLabel(QTextStream &out, const VLayoutPiece &detail, const QVector<QPointF> &labelShape,
                   const VTextManager &tm);
    void PlotLabelSVGFont(QTextStream &out, const VLayoutPiece &detail, const QVector<QPointF> &labelShape,
                          const VTextManager &tm);
    void PlotLabelOutlineFont(QTextStream &out, const VLayoutPiece &detail, const QVector<QPointF> &labelShape,
                              const VTextManager &tm);

    template <class T> auto ConvertPath(const QVector<T> &path) const -> QVector<T>;
    template <class T> auto ConvertPoint(T point) const -> T;
    void PlotPath(QTextStream &out, QVector<QPoint> path, Qt::PenStyle penStyle);
    void PlotSolidLinePath(QTextStream &out, QVector<QPoint> path);
    void PlotPathForStyle(QTextStream &out, QVector<QPoint> path, QVector<int> pattern);
    void PlotPainterPath(QTextStream &out, const QPainterPath &path, Qt::PenStyle penStyle);

    void HPPenUp(QTextStream &out, QPoint point);
    void HPPenUp(QTextStream &out);

    void HPPenDown(QTextStream &out, QPoint point);
    void HPPenDown(QTextStream &out);

    auto PatternForStyle(Qt::PenStyle style) const -> QVector<int>;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VHPGLEngine::isActive() const -> bool
{
    return m_active;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VHPGLEngine::setActive(bool newState)
{
    m_active = newState;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VHPGLEngine::GetSize() const -> QSize
{
    return m_size;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VHPGLEngine::SetSize(QSize size)
{
    Q_ASSERT(not isActive());
    m_size = size;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VHPGLEngine::GetFileName() const -> QString
{
    return m_fileName;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VHPGLEngine::SetFileName(const QString &filename)
{
    Q_ASSERT(not isActive());
    m_fileName = filename;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VHPGLEngine::SetIsertNewLine(bool insert)
{
    Q_ASSERT(not isActive());
    m_inserNewLine = insert;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VHPGLEngine::GetSingleLineFont() const -> bool
{
    return m_singleLineFont;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VHPGLEngine::SetSingleLineFont(bool singleLineFont)
{
    Q_ASSERT(not isActive());
    m_singleLineFont = singleLineFont;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VHPGLEngine::GetSingleStrokeOutlineFont() const -> bool
{
    return m_singleStrokeOutlineFont;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VHPGLEngine::SetSingleStrokeOutlineFont(bool singleStrokeOutlineFont)
{
    Q_ASSERT(not isActive());
    m_singleStrokeOutlineFont = singleStrokeOutlineFont;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VHPGLEngine::GetPenWidth() const -> int
{
    return m_penWidthPx;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VHPGLEngine::SetPenWidth(int newPenWidth)
{
    m_penWidthPx = newPenWidth;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VHPGLEngine::GetXScale() const -> qreal
{
    return m_xscale;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VHPGLEngine::SetXScale(const qreal &xscale)
{
    Q_ASSERT(not isActive());
    m_xscale = xscale;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VHPGLEngine::GetYScale() const -> qreal
{
    return m_yscale;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VHPGLEngine::SetYScale(const qreal &yscale)
{
    Q_ASSERT(not isActive());
    m_yscale = yscale;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VHPGLEngine::GetShowGrainline() const -> bool
{
    return m_showGrainline;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VHPGLEngine::SetShowGrainline(bool newShowGrainline)
{
    Q_ASSERT(not isActive());
    m_showGrainline = newShowGrainline;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VHPGLEngine::SetBoundaryTogetherWithNotches(bool value)
{
    Q_ASSERT(not isActive());
    m_togetherWithNotches = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VHPGLEngine::IsBoundaryTogetherWithNotches() const -> bool
{
    return m_togetherWithNotches;
}

#endif // VHPGLENGINE_H
