/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef VABSTRACTPIECE_P_H
#define VABSTRACTPIECE_P_H

#include <QCoreApplication>
#include <QSharedData>
#include <QString>
#include <QUuid>

#include "../ifc/exception/vexception.h"
#include "../vmisc/defglobal.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VAbstractPieceData : public QSharedData
{
    Q_DECLARE_TR_FUNCTIONS(VAbstractPieceData) // NOLINT

public:
    VAbstractPieceData() = default;
    VAbstractPieceData(const VAbstractPieceData &piece) = default;
    ~VAbstractPieceData() = default;

    friend auto operator<<(QDataStream &dataStream, const VAbstractPieceData &piece) -> QDataStream &;
    friend auto operator>>(QDataStream &dataStream, VAbstractPieceData &piece) -> QDataStream &;

    QString m_name{tr("Detail")}; // NOLINT (misc-non-private-member-variables-in-classes)
    /** @brief forbidFlipping forbid piece to be mirrored in a layout. */
    bool m_forbidFlipping{false};                            // NOLINT (misc-non-private-member-variables-in-classes)
    bool m_forceFlipping{false};                             // NOLINT (misc-non-private-member-variables-in-classes)
    bool m_symmetricalCopy{false};                           // NOLINT (misc-non-private-member-variables-in-classes)
    bool m_followGrainline{false};                           // NOLINT (misc-non-private-member-variables-in-classes)
    bool m_seamAllowance{false};                             // NOLINT (misc-non-private-member-variables-in-classes)
    bool m_seamAllowanceBuiltIn{false};                      // NOLINT (misc-non-private-member-variables-in-classes)
    bool m_hideMainPath{false};                              // NOLINT (misc-non-private-member-variables-in-classes)
    qreal m_width{0};                                        // NOLINT (misc-non-private-member-variables-in-classes)
    qreal m_mx{0};                                           // NOLINT (misc-non-private-member-variables-in-classes)
    qreal m_my{0};                                           // NOLINT (misc-non-private-member-variables-in-classes)
    uint m_priority{0};                                      // NOLINT (misc-non-private-member-variables-in-classes)
    QUuid m_uuid{QUuid::createUuid()};                       // NOLINT (misc-non-private-member-variables-in-classes)
    bool m_onDrawing{false};                                 // NOLINT (misc-non-private-member-variables-in-classes)
    bool m_showFullPiece{true};                              // NOLINT (misc-non-private-member-variables-in-classes)
    unsigned int m_foldLineSvgFontSize{defFoldLineFontSize}; // NOLINT (misc-non-private-member-variables-in-classes)
    bool m_foldLineLabelFontItalic{false};                   // NOLINT (misc-non-private-member-variables-in-classes)
    bool m_foldLineLabelFontBold{false};                     // NOLINT (misc-non-private-member-variables-in-classes)
    QString m_foldLineLabel{};                               // NOLINT (misc-non-private-member-variables-in-classes)
    int m_foldLineLabelAlignment{Qt::AlignHCenter};          // NOLINT (misc-non-private-member-variables-in-classes)
    bool m_showMirrorLine{true};                             // NOLINT (misc-non-private-member-variables-in-classes)

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    FoldLineType m_foldLineType{FoldLineType::TwoArrowsTextAbove};

private:
    Q_DISABLE_ASSIGN_MOVE(VAbstractPieceData) // NOLINT

    static constexpr quint32 streamHeader = 0x05CDD73A; // CRC-32Q string "VAbstractPieceData"
    static constexpr quint16 classVersion = 8;
};

QT_WARNING_POP

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
inline auto operator<<(QDataStream &dataStream, const VAbstractPieceData &piece) -> QDataStream &
{
    dataStream << VAbstractPieceData::streamHeader << VAbstractPieceData::classVersion;

    // Added in classVersion = 1
    dataStream << piece.m_name;
    dataStream << piece.m_forbidFlipping;
    dataStream << piece.m_forceFlipping;
    dataStream << piece.m_seamAllowance;
    dataStream << piece.m_seamAllowanceBuiltIn;
    dataStream << piece.m_hideMainPath;
    dataStream << piece.m_width;
    dataStream << piece.m_mx;
    dataStream << piece.m_my;

    // Added in classVersion = 2
    dataStream << piece.m_priority;

    // Added in classVersion = 3
    dataStream << piece.m_uuid;

    // Added in classVersion = 4
    dataStream << piece.m_onDrawing;

    // Added in classVersion = 5
    dataStream << piece.m_followGrainline;

    // Added in classVersion = 6
    dataStream << piece.m_showFullPiece;
    dataStream << piece.m_foldLineSvgFontSize;
    dataStream << piece.m_foldLineType;
    dataStream << piece.m_foldLineLabelFontItalic;
    dataStream << piece.m_foldLineLabelFontBold;
    dataStream << piece.m_foldLineLabel;
    dataStream << piece.m_foldLineLabelAlignment;

    // Added in classVersion = 7
    dataStream << piece.m_showMirrorLine;

    // Added in classVersion = 8
    dataStream << piece.m_symmetricalCopy;

    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto operator>>(QDataStream &dataStream, VAbstractPieceData &piece) -> QDataStream &
{
    quint32 actualStreamHeader = 0;
    dataStream >> actualStreamHeader;

    if (actualStreamHeader != VAbstractPieceData::streamHeader)
    {
        QString const message =
            QCoreApplication::tr("VAbstractPieceData prefix mismatch error: actualStreamHeader = 0x%1 "
                                 "and streamHeader = 0x%2")
                .arg(actualStreamHeader, 8, 0x10, QLatin1Char('0'))
                .arg(VAbstractPieceData::streamHeader, 8, 0x10, QLatin1Char('0'));
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    dataStream >> actualClassVersion;

    if (actualClassVersion > VAbstractPieceData::classVersion)
    {
        QString const message =
            QCoreApplication::tr("VAbstractPieceData compatibility error: actualClassVersion = %1 and "
                                 "classVersion = %2")
                .arg(actualClassVersion)
                .arg(VAbstractPieceData::classVersion);
        throw VException(message);
    }

    dataStream >> piece.m_name;
    dataStream >> piece.m_forbidFlipping;
    dataStream >> piece.m_forceFlipping;
    dataStream >> piece.m_seamAllowance;
    dataStream >> piece.m_seamAllowanceBuiltIn;
    dataStream >> piece.m_hideMainPath;
    dataStream >> piece.m_width;
    dataStream >> piece.m_mx;
    dataStream >> piece.m_my;

    if (actualClassVersion >= 2)
    {
        dataStream >> piece.m_priority;
    }

    if (actualClassVersion >= 3)
    {
        dataStream >> piece.m_uuid;
    }

    if (actualClassVersion >= 4)
    {
        dataStream >> piece.m_onDrawing;
    }

    if (actualClassVersion >= 5)
    {
        dataStream >> piece.m_followGrainline;
    }

    if (actualClassVersion >= 6)
    {
        dataStream >> piece.m_showFullPiece;
        dataStream >> piece.m_foldLineSvgFontSize;
        dataStream >> piece.m_foldLineType;
        dataStream >> piece.m_foldLineLabelFontItalic;
        dataStream >> piece.m_foldLineLabelFontBold;
        dataStream >> piece.m_foldLineLabel;
        dataStream >> piece.m_foldLineLabelAlignment;
    }

    if (actualClassVersion >= 7)
    {
        dataStream >> piece.m_showMirrorLine;
    }

    if (actualClassVersion >= 8)
    {
        dataStream >> piece.m_symmetricalCopy;
    }

    return dataStream;
}

#endif // VABSTRACTPIECE_P_H
