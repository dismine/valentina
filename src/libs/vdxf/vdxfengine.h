/************************************************************************
 **
 **  @file   vdxfengine.h
 **  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
 **  @date   12 8, 2015
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

#ifndef VDXFENGINE_H
#define VDXFENGINE_H

#include <QPaintEngine>
#include <QPointF>
#include <QRectF>
#include <QSize>
#include <QString>
#include <QtGlobal>
#include <string>

#include "../vmisc/def.h"
#include "../vmisc/defglobal.h"
#include "dxfdef.h"
#include "libdxfrw/drw_base.h"

class QTextStream;
class dx_iface;
class DRW_Text;
class VLayoutPiece;
class DRW_Entity;
class dx_ifaceBlock;
class VLayoutPoint;
class DRW_Point;
class DRW_ASTMNotch;
struct VLayoutPassmark;
class DRW_ATTDEF;
class DRW_Circle;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class VTextCodec;
#else
class QTextCodec;
using VTextCodec = QTextCodec;
#endif

class VDxfEngine final : public QPaintEngine
{
    friend class VDxfPaintDevice;

public:
    using QPaintEngine::drawEllipse;
    using QPaintEngine::drawLines;
    using QPaintEngine::drawPolygon;

    VDxfEngine();
    ~VDxfEngine() override;

    auto type() const -> Type override;

    auto begin(QPaintDevice *pdev) -> bool override;
    auto end() -> bool override;

    void updateState(const QPaintEngineState &state) override;

    void drawPath(const QPainterPath &path) override;
    void drawLines(const QLineF *lines, int lineCount) override;
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;
    void drawEllipse(const QRectF &rect) override;
    void drawTextItem(const QPointF &p, const QTextItem &textItem) override;
    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override;

    auto GetSize() const -> QSize;
    void SetSize(const QSize &value);

    auto GetResolution() const -> double;
    void SetResolution(double value);

    auto GetFileName() const -> QString;
    void SetFileName(const QString &value);

    auto GetVersion() const -> DRW::Version;
    void SetVersion(DRW::Version version);

    void SetBinaryFormat(bool binary);
    auto IsBinaryFormat() const -> bool;

    auto GetPenStyle() -> std::string;
    auto GetPenColor() -> int;

    void SetMeasurement(const VarMeasurement &var);
    void SetInsunits(const VarInsunits &var);

    auto GetXScale() const -> qreal;
    void SetXScale(const qreal &xscale);

    auto GetYScale() const -> qreal;
    void SetYScale(const qreal &yscale);

    void SetBoundaryTogetherWithNotches(bool value);
    auto IsBoundaryTogetherWithNotches() const -> bool;

    auto DxfApparelCompatibility() const -> DXFApparelCompatibility;
    void SetDxfApparelCompatibility(DXFApparelCompatibility mode);

    auto ErrorString() const -> QString;

private:
    Q_DISABLE_COPY_MOVE(VDxfEngine) // NOLINT

    QSize m_size{};
    double m_resolution{PrintDPI};
    QString m_fileName{};
    DRW::Version m_version{DRW::AC1009};
    bool m_binary{false};
    QTransform m_matrix{};
    QSharedPointer<dx_iface> m_input{};
    VarMeasurement m_varMeasurement{VarMeasurement::Metric};
    VarInsunits m_varInsunits{VarInsunits::Millimeters};
    DRW_Text *m_textBuffer{nullptr};
    qreal m_xscale{1};
    qreal m_yscale{1};
    bool m_togetherWithNotches{false};
    DXFApparelCompatibility m_compatibilityMode{DXFApparelCompatibility::STANDARD};

    Q_REQUIRED_RESULT auto FromPixel(double pix, const VarInsunits &unit) const -> double;
    Q_REQUIRED_RESULT auto ToPixel(double val, const VarInsunits &unit) const -> double;

    auto ExportToAAMA(const QVector<VLayoutPiece> &details) -> bool;
    void ExportAAMAOutline(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportAAMADraw(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportAAMADrawSewLine(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportAAMADrawInternalPaths(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportAAMADrawPlaceLabels(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportAAMAIntcut(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportAAMANotch(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportAAMAGrainline(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportPieceText(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportStyleSystemText(const QSharedPointer<dx_iface> &input, const QVector<VLayoutPiece> &details);
    void ExportAAMADrill(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportAAMADrawFoldLine(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);

    auto ExportToASTM(const QVector<VLayoutPiece> &details) -> bool;
    void ExportASTMPieceBoundary(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportASTMSewLine(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportASTMDrawInternalPaths(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportASTMDrawPlaceLabels(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportASTMInternalCutout(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportASTMAnnotationText(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportASTMDrill(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportASTMNotches(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportASTMMirrorLine(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);
    void ExportASTMDrawFoldLine(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail);

    void ExportAnnotationText(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail,
                              const UTF8STRING &layer);

    Q_REQUIRED_RESULT auto ExportASTMNotch(const VLayoutPassmark &passmark) -> DRW_ASTMNotch *;
    Q_REQUIRED_RESULT auto ExportASTMNotchDataDependecy(const VLayoutPassmark &passmark, const UTF8STRING &notchLayer,
                                                        const VLayoutPiece &detail) -> DRW_ATTDEF *;

    void ExportTurnPoints(const QSharedPointer<dx_ifaceBlock> &detailBlock, const QVector<VLayoutPoint> &points) const;
    void ExportCurvePoints(const QSharedPointer<dx_ifaceBlock> &detailBlock, const QVector<VLayoutPoint> &points) const;

    Q_REQUIRED_RESULT auto AAMAPolygon(const QVector<VLayoutPoint> &polygon, const UTF8STRING &layer, bool forceClosed)
        -> DRW_Entity *;
    Q_REQUIRED_RESULT auto AAMALine(const QLineF &line, const UTF8STRING &layer) -> DRW_Entity *;
    Q_REQUIRED_RESULT auto AAMAText(const QPointF &pos, const QString &text, const UTF8STRING &layer) -> DRW_Entity *;
    Q_REQUIRED_RESULT auto AAMAPoint(const QPointF &pos, const UTF8STRING &layer) const -> DRW_Point *;
    Q_REQUIRED_RESULT auto AAMACircle(const QPointF &pos, const UTF8STRING &layer, qreal radius) const -> DRW_Circle *;

    template <class P, class V, class C>
    Q_REQUIRED_RESULT auto CreateAAMAPolygon(const QVector<C> &polygon, const UTF8STRING &layer, bool forceClosed)
        -> P *;

    static auto FromUnicodeToCodec(const QString &str, VTextCodec *codec) -> std::string;
    auto GetFileNameForLocale() const -> std::string;

    static auto NotchPrecedingPoint(const QVector<VLayoutPoint> &boundary, QPointF notchBase, QPointF &point) -> bool;

    void AAMADrawFoldLineTwoArrows(const QVector<QVector<QPointF>> &points,
                                   const QSharedPointer<dx_ifaceBlock> &detailBlock);
    void AAMADrawFoldLineThreeDots(const QVector<QVector<QPointF>> &points,
                                   const QSharedPointer<dx_ifaceBlock> &detailBlock, qreal radius);
    void AAMADrawFoldLineThreeX(const QVector<QVector<QPointF>> &points,
                                const QSharedPointer<dx_ifaceBlock> &detailBlock);

    void ASTMDrawFoldLineTwoArrows(const QVector<QVector<QPointF>> &points,
                                   const QSharedPointer<dx_ifaceBlock> &detailBlock);

    static void CheckLabelCompatibilityRPCADV09(const VLayoutPiece &detail);
    static void CheckLabelCompatibilityRPCADV08(const VLayoutPiece &detail);

    auto PieceOutline(const VLayoutPiece &detail) const -> QVector<VLayoutPoint>;
};

#endif // VDXFENGINE_H
