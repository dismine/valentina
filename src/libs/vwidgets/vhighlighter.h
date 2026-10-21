/************************************************************************
 **
 **  @file   vhighlighter.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 11, 2017
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
#ifndef VHIGHLIGHTER_H
#define VHIGHLIGHTER_H

#include <QSet>
#include <QStringList>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

#include "../vmisc/defglobal.h"

class QString;
class QTextDocument;

struct ParenthesisInfo
{
    char character;
    vsizetype position;
};

class VTextBlockData final : public QTextBlockUserData
{
public:
    VTextBlockData();

    auto Parentheses() -> QVector<ParenthesisInfo *>;
    void insert(ParenthesisInfo *info);

private:
    QVector<ParenthesisInfo *> m_parentheses;
};

class VHighlighter : public QSyntaxHighlighter
{
public:
    explicit VHighlighter(QTextDocument *document);

    void SetFormulaHighlightingEnabled(bool enabled);
    auto IsFormulaHighlightingEnabled() const -> bool;

    // Rebuild colors and translated names after a theme or language change.
    void Refresh();

protected:
    void highlightBlock(const QString &text) override;

private:
    bool m_formulaHighlighting{true};

    QSet<QString> m_locals{};
    QStringList m_measurementPrefixes{};
    QStringList m_geometryPrefixes{};
    QSet<QString> m_exactGeometry{};
    QSet<QString> m_functions{};

    QTextCharFormat m_localFormat{};
    QTextCharFormat m_patternFormat{};
    QTextCharFormat m_measurementFormat{};
    QTextCharFormat m_geometryFormat{};
    QTextCharFormat m_functionFormat{};
    QTextCharFormat m_numberFormat{};
    QTextCharFormat m_ternaryFormat{};
    QTextCharFormat m_comparisonFormat{};
    QTextCharFormat m_arithmeticFormat{};

    void RebuildFormats();
    void RebuildLexicon();
    void UpdateLocals();
    void CollectParentheses(const QString &text);
    void HighlightFormula(const QString &text);

    auto FormatForIdentifier(const QString &identifier, const QString &line, vsizetype end) const
        -> const QTextCharFormat *;
};

#endif // VHIGHLIGHTER_H
