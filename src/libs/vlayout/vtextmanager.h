/************************************************************************
 **
 **  @file   vpatternpiecedata.cpp
 **  @author Bojan Kverh
 **  @date   July 19, 2016
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

#ifndef VTEXTMANAGER_H
#define VTEXTMANAGER_H

#include <QDate>
#include <QFont>
#include <QList>
#include <QString>
#include <QStringList>
#include <Qt>
#include <QtGlobal>
#include <QCoreApplication>
#include <QVector>

class VPieceLabelData;
class VAbstractPattern;
class VContainer;

#define MIN_FONT_SIZE               5
#define MAX_FONT_SIZE               128

/**
 * @brief The TextLine struct holds the information about one text line
 */
struct TextLine
{
    QString       m_qsText{};
    int           m_iFontSize{MIN_FONT_SIZE};  // 0 means default
    bool          m_bold{false};
    bool          m_italic{false};
    Qt::Alignment m_eAlign{Qt::AlignCenter};

    friend auto operator<<(QDataStream& dataStream, const TextLine& data) -> QDataStream&;
    friend auto operator>>(QDataStream& dataStream, TextLine& data) -> QDataStream&;
private:
    static const quint32 streamHeader;
    static const quint16 classVersion;
};

/**
 * @brief The VTextManager class this class is used to determine whether a collection of
 * text lines can fit into specified bounding box and with what font size
 */
class VTextManager
{
    Q_DECLARE_TR_FUNCTIONS(VTextManager)
public:
    VTextManager();
    virtual ~VTextManager() = default;

    VTextManager(const VTextManager &text);
    VTextManager &operator=(const VTextManager &text);

    virtual auto GetSpacing() const -> int;

    void SetFont(const QFont& font);
    auto GetFont() const -> const QFont&;
    void SetFontSize(int iFS);
    void FitFontSize(qreal fW, qreal fH);

    auto GetAllSourceLines() const -> QVector<TextLine>;
    void SetAllSourceLines(const QVector<TextLine> &lines);
    auto GetSourceLinesCount() const -> int;
    auto GetSourceLine(int i) const -> const TextLine&;

    void Update(const QString& qsName, const VPieceLabelData& data, const VContainer *pattern);
    void Update(VAbstractPattern* pDoc, const VContainer *pattern);

    friend auto operator<<(QDataStream& dataStream, const VTextManager& data) -> QDataStream&;
    friend auto operator>>(QDataStream& dataStream, VTextManager& data) -> QDataStream&;

private:
    QFont             m_font;
    QVector<TextLine> m_liLines;

    static QVector<TextLine> m_patternLabelLines;
    static const quint32 streamHeader;
    static const quint16 classVersion;
};

#endif // VTEXTMANAGER_H
