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

#include <qcompilerdetection.h>
#include <QMatrix>
#include <QPaintEngine>
#include <QPointF>
#include <QRectF>
#include <QSize>
#include <QString>
#include <QtGlobal>
#include <string>

#include "../vmisc/def.h"
#include "dxfdef.h"
#include "libdxfrw/drw_base.h"

class QTextStream;
class dx_iface;
class DRW_Text;
class VLayoutPiece;
class DRW_Entity;
class dx_ifaceBlock;

class VDxfEngine final : public QPaintEngine
{
    friend class VDxfPaintDevice;
public:
    VDxfEngine();
    ~VDxfEngine() override;

    auto type() const -> Type override;

    auto begin(QPaintDevice *pdev) -> bool override;
    auto end() -> bool override;

    void updateState(const QPaintEngineState &state) override;

    void drawPath(const QPainterPath &path) override;
    void drawLines(const QLineF * lines, int lineCount) override;
    void drawLines(const QLine * lines, int lineCount) override;
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;
    void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode) override;
    void drawEllipse(const QRectF & rect) override;
    void drawEllipse(const QRect & rect) override;
    void drawTextItem(const QPointF & p, const QTextItem & textItem) override;
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

    auto ErrorString() const -> QString;

private:
    Q_DISABLE_COPY_MOVE(VDxfEngine) // NOLINT

    QSize m_size{};
    double m_resolution{static_cast<int>(PrintDPI)};
    QString m_fileName{};
    DRW::Version m_version{DRW::AC1014};
    bool m_binary{false};
    QTransform m_matrix{};
    QSharedPointer<dx_iface> m_input{};
    VarMeasurement m_varMeasurement{VarMeasurement::Metric};
    VarInsunits m_varInsunits{VarInsunits::Millimeters};
    DRW_Text *m_textBuffer{nullptr};
    qreal m_xscale{1};
    qreal m_yscale{1};

    Q_REQUIRED_RESULT auto FromPixel(double pix, const VarInsunits &unit) const -> double;
    Q_REQUIRED_RESULT auto ToPixel(double val, const VarInsunits &unit) const -> double;

    auto ExportToAAMA(const QVector<VLayoutPiece> &details) -> bool;
    void ExportAAMAOutline(dx_ifaceBlock *detailBlock, const VLayoutPiece &detail);
    void ExportAAMADraw(dx_ifaceBlock *detailBlock, const VLayoutPiece &detail);
    void ExportAAMAIntcut(dx_ifaceBlock *detailBlock, const VLayoutPiece &detail);
    void ExportAAMANotch(dx_ifaceBlock *detailBlock, const VLayoutPiece &detail);
    void ExportAAMAGrainline(dx_ifaceBlock *detailBlock, const VLayoutPiece &detail);
    void ExportPieceText(dx_ifaceBlock *detailBlock, const VLayoutPiece &detail);
    void ExportStyleSystemText(const QSharedPointer<dx_iface> &input, const QVector<VLayoutPiece> &details);
    void ExportAAMADrill(dx_ifaceBlock *detailBlock, const VLayoutPiece &detail);

    auto ExportToASTM(const QVector<VLayoutPiece> &details) -> bool;
    void ExportASTMPieceBoundary(dx_ifaceBlock *detailBlock, const VLayoutPiece &detail);
    void ExportASTMSewLine(dx_ifaceBlock *detailBlock, const VLayoutPiece &detail);
    void ExportASTMInternalLine(dx_ifaceBlock *detailBlock, const VLayoutPiece &detail);
    void ExportASTMInternalCutout(dx_ifaceBlock *detailBlock, const VLayoutPiece &detail);
    void ExportASTMAnnotationText(dx_ifaceBlock *detailBlock, const VLayoutPiece &detail);
    void ExportASTMDrill(dx_ifaceBlock *detailBlock, const VLayoutPiece &detail);
    void ExportASTMNotch(dx_ifaceBlock *detailBlock, const VLayoutPiece &detail);

    Q_REQUIRED_RESULT auto AAMAPolygon(const QVector<QPointF> &polygon, const UTF8STRING &layer,
                                       bool forceClosed) -> DRW_Entity *;
    Q_REQUIRED_RESULT auto AAMALine(const QLineF &line, const UTF8STRING &layer) -> DRW_Entity *;
    Q_REQUIRED_RESULT auto AAMAText(const QPointF &pos, const QString &text, const UTF8STRING &layer) -> DRW_Entity *;

    template<class P, class V>
    Q_REQUIRED_RESULT auto CreateAAMAPolygon(const QVector<QPointF> &polygon, const UTF8STRING &layer,
                                             bool forceClosed) -> P *;

    static auto FromUnicodeToCodec(const QString &str, QTextCodec *codec) -> std::string;
    auto GetFileNameForLocale() const -> std::string;
};

#endif // VDXFENGINE_H
