/************************************************************************
 **
 **  @file   vhighlighter.cpp
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
#include "vhighlighter.h"

#include <algorithm>
#include <memory>

#include <QFont>
#include <QTextDocument>

#include "../ifc/ifcdef.h"
#include "../vmisc/literals.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vtranslatevars.h"

namespace
{
constexpr int BlockStateNormal = 0;
constexpr int BlockStateInString = 1;

//---------------------------------------------------------------------------------------------------------------------
auto IsIdentStart(QChar c) -> bool
{
    return c == '#' || c == '@' || c == '_' || c.isLetter();
}

//---------------------------------------------------------------------------------------------------------------------
auto IsIdentContinuation(QChar c) -> bool
{
    return c == '_' || c == '#' || c == '@' || c == '\'' || c.isLetter() || c.isDigit();
}

//---------------------------------------------------------------------------------------------------------------------
// True when `pos` (just after an identifier) is a real assignment, not ==, <=, >=, or !=.
auto IsAssignmentAt(const QString &text, vsizetype pos) -> bool
{
    const vsizetype size = text.size();
    while (pos < size && text.at(pos).isSpace())
    {
        ++pos;
    }

    if (pos >= size || text.at(pos) != '=')
    {
        return false;
    }

    return pos + 1 >= size || text.at(pos + 1) != '=';
}

//---------------------------------------------------------------------------------------------------------------------
auto ReadIdentifierEnd(const QString &text, vsizetype pos) -> vsizetype
{
    const vsizetype size = text.size();
    if (pos >= size || not IsIdentStart(text.at(pos)))
    {
        return pos;
    }

    vsizetype i = pos + 1;
    while (i < size && IsIdentContinuation(text.at(i)))
    {
        ++i;
    }

    return i;
}

//---------------------------------------------------------------------------------------------------------------------
auto ReadNumberEnd(const QString &text, vsizetype pos) -> vsizetype
{
    const vsizetype size = text.size();
    if (pos >= size)
    {
        return pos;
    }

    vsizetype i = pos;
    if (text.at(i) == '.')
    {
        if (i + 1 >= size || not text.at(i + 1).isDigit())
        {
            return pos;
        }
        ++i;
    }
    else if (not text.at(i).isDigit())
    {
        return pos;
    }

    while (i < size && text.at(i).isDigit())
    {
        ++i;
    }

    // One decimal separator. Argument lists use ';', so a comma here is a decimal or grouping mark.
    if (i < size && (text.at(i) == '.' || text.at(i) == ',') && i + 1 < size && text.at(i + 1).isDigit())
    {
        ++i;
        while (i < size && text.at(i).isDigit())
        {
            ++i;
        }
    }

    if (i < size && (text.at(i) == 'e' || text.at(i) == 'E'))
    {
        vsizetype exp = i + 1;
        if (exp < size && (text.at(exp) == '+' || text.at(exp) == '-'))
        {
            ++exp;
        }
        if (exp < size && text.at(exp).isDigit())
        {
            i = exp;
            while (i < size && text.at(i).isDigit())
            {
                ++i;
            }
        }
    }

    return i;
}

//---------------------------------------------------------------------------------------------------------------------
auto StartsWithAny(const QString &text, const QStringList &prefixes) -> bool
{
    for (const QString &prefix : prefixes)
    {
        if (not prefix.isEmpty() && text.startsWith(prefix))
        {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void AppendUnique(QStringList &list, const QString &value)
{
    if (not value.isEmpty() && not list.contains(value))
    {
        list.append(value);
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VTextBlockData::VTextBlockData()
  : m_parentheses()
{
    // Nothing to do
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextBlockData::Parentheses() -> QVector<ParenthesisInfo *>
{
    return m_parentheses;
}

//---------------------------------------------------------------------------------------------------------------------
void VTextBlockData::insert(ParenthesisInfo *info)
{
    int i = 0;
    while (i < m_parentheses.size() && info->position > m_parentheses.at(i)->position)
    {
        ++i;
    }

    m_parentheses.insert(i, info);
}

//---------------------------------------------------------------------------------------------------------------------
VHighlighter::VHighlighter(QTextDocument *document)
  : QSyntaxHighlighter(document)
{
    RebuildFormats();
    RebuildLexicon();
    UpdateLocals();

    if (document != nullptr)
    {
        // Locals are declared anywhere in the formula, so a change on line 1 has to recolor later lines.
        // Qt highlights the dirty blocks before this slot runs; a changed declaration set repaints the rest.
        connect(document,
                &QTextDocument::contentsChange,
                this,
                [this](int /*position*/, int /*charsRemoved*/, int /*charsAdded*/)
                {
                    if (not m_formulaHighlighting)
                    {
                        return;
                    }

                    const QSet<QString> previous = m_locals;
                    UpdateLocals();
                    if (previous != m_locals)
                    {
                        rehighlight();
                    }
                });
    }

    if (m_formulaHighlighting && document != nullptr && not document->isEmpty())
    {
        rehighlight();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHighlighter::SetFormulaHighlightingEnabled(bool enabled)
{
    if (m_formulaHighlighting == enabled)
    {
        return;
    }

    m_formulaHighlighting = enabled;
    if (enabled)
    {
        RebuildFormats();
        RebuildLexicon();
        UpdateLocals();
    }

    rehighlight();
}

//---------------------------------------------------------------------------------------------------------------------
auto VHighlighter::IsFormulaHighlightingEnabled() const -> bool
{
    return m_formulaHighlighting;
}

//---------------------------------------------------------------------------------------------------------------------
void VHighlighter::Refresh()
{
    RebuildFormats();
    RebuildLexicon();
    if (m_formulaHighlighting)
    {
        UpdateLocals();
        rehighlight();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHighlighter::highlightBlock(const QString &text)
{
    CollectParentheses(text);

    if (m_formulaHighlighting)
    {
        HighlightFormula(text);
    }
    else
    {
        setCurrentBlockState(BlockStateNormal);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHighlighter::RebuildFormats()
{
    const bool dark = VTheme::IsInDarkTheme();

    // Gruvbox. Ternary is the same orange as comparisons, in bold, so ? and : stay readable.
    m_localFormat.setForeground(dark ? QColor(0x8e, 0xc0, 0x7c) : QColor(0x42, 0x7b, 0x58));
    m_patternFormat.setForeground(dark ? QColor(0x83, 0xa5, 0x98) : QColor(0x07, 0x66, 0x78));
    m_measurementFormat.setForeground(dark ? QColor(0xfa, 0xbd, 0x2f) : QColor(0xb5, 0x76, 0x14));
    m_geometryFormat.setForeground(dark ? QColor(0xa8, 0x99, 0x84) : QColor(0x7c, 0x6f, 0x64));
    m_functionFormat.setForeground(dark ? QColor(0xb8, 0xbb, 0x26) : QColor(0x79, 0x74, 0x0e));
    m_numberFormat.setForeground(dark ? QColor(0xd3, 0x86, 0x9b) : QColor(0x8f, 0x3f, 0x71));
    m_ternaryFormat.setForeground(dark ? QColor(0xfe, 0x80, 0x19) : QColor(0xaf, 0x3a, 0x03));
    m_ternaryFormat.setFontWeight(QFont::Bold);
    m_comparisonFormat.setForeground(dark ? QColor(0xfe, 0x80, 0x19) : QColor(0xaf, 0x3a, 0x03));
    m_arithmeticFormat.setForeground(dark ? QColor(0xd6, 0x5d, 0x0e) : QColor(0x9d, 0x00, 0x06));
}

//---------------------------------------------------------------------------------------------------------------------
void VHighlighter::RebuildLexicon()
{
    m_measurementPrefixes.clear();
    m_geometryPrefixes.clear();
    m_exactGeometry.clear();
    m_functions.clear();

    const VTranslateVars *trVars = nullptr;
    if (VAbstractApplication::VApp() != nullptr)
    {
        trVars = VAbstractApplication::VApp()->TrVars();
    }

    const auto toUser = [trVars](const QString &name) -> QString
    {
        if (trVars == nullptr)
        {
            return name;
        }

        return trVars->InternalVarToUser(name);
    };

    AppendUnique(m_measurementPrefixes, measurement_);
    AppendUnique(m_measurementPrefixes, toUser(measurement_));
    std::sort(m_measurementPrefixes.begin(),
              m_measurementPrefixes.end(),
              [](const QString &a, const QString &b) { return a.size() > b.size(); });

    const QStringList builtins = BuilInVariables();
    for (const QString &name : builtins)
    {
        if (name == measurement_)
        {
            continue;
        }

        const QString user = toUser(name);
        if (name == currentLength || name == currentSeamAllowance)
        {
            m_exactGeometry.insert(name);
            if (not user.isEmpty())
            {
                m_exactGeometry.insert(user);
            }
            continue;
        }

        AppendUnique(m_geometryPrefixes, name);
        AppendUnique(m_geometryPrefixes, user);
    }

    std::sort(m_geometryPrefixes.begin(),
              m_geometryPrefixes.end(),
              [](const QString &a, const QString &b) { return a.size() > b.size(); });

    const QStringList canonical = BuilInFunctions();
    for (const QString &name : canonical)
    {
        m_functions.insert(name);
    }
    m_functions.insert(QStringLiteral("atan2"));

    if (trVars != nullptr)
    {
        const QString locale = VAbstractApplication::VApp()->Settings()->GetLocale();
        const auto functions = trVars->GetFunctions();
        for (auto it = functions.cbegin(); it != functions.cend(); ++it)
        {
            m_functions.insert(it.key());
            const QString translated = it.value().translate(locale);
            if (not translated.isEmpty())
            {
                m_functions.insert(translated);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHighlighter::UpdateLocals()
{
    QSet<QString> locals;
    if (document() == nullptr)
    {
        m_locals = locals;
        return;
    }

    const QString text = document()->toPlainText();
    const vsizetype size = text.size();
    bool inString = false;

    for (vsizetype i = 0; i < size; ++i)
    {
        const QChar c = text.at(i);
        if (inString)
        {
            if (c == '\\' && i + 1 < size)
            {
                ++i;
                continue;
            }
            if (c == '"')
            {
                inString = false;
            }
            continue;
        }

        if (c == '"')
        {
            inString = true;
            continue;
        }

        const vsizetype end = ReadIdentifierEnd(text, i);
        if (end == i)
        {
            continue;
        }

        const QString identifier = text.mid(i, end - i);
        if (identifier.startsWith(CustomIncrSign) && identifier.size() > 1 && IsAssignmentAt(text, end))
        {
            locals.insert(identifier);
        }

        i = end - 1;
    }

    m_locals = locals;
}

//---------------------------------------------------------------------------------------------------------------------
void VHighlighter::CollectParentheses(const QString &text)
{
    auto data = std::make_unique<VTextBlockData>();

    vsizetype leftPos = text.indexOf('(');
    while (leftPos != -1)
    {
        auto info = std::make_unique<ParenthesisInfo>();
        info->character = '(';
        info->position = leftPos;

        data->insert(info.release());
        leftPos = text.indexOf('(', leftPos + 1);
    }

    vsizetype rightPos = text.indexOf(')');
    while (rightPos != -1)
    {
        auto info = std::make_unique<ParenthesisInfo>();
        info->character = ')';
        info->position = rightPos;

        data->insert(info.release());

        rightPos = text.indexOf(')', rightPos + 1);
    }

    setCurrentBlockUserData(data.release());
}

//---------------------------------------------------------------------------------------------------------------------
void VHighlighter::HighlightFormula(const QString &text)
{
    const vsizetype size = text.size();
    bool inString = previousBlockState() == BlockStateInString;

    for (vsizetype i = 0; i < size;)
    {
        const QChar c = text.at(i);
        if (inString)
        {
            if (c == '\\' && i + 1 < size)
            {
                i += 2;
                continue;
            }
            if (c == '"')
            {
                inString = false;
            }
            ++i;
            continue;
        }

        if (c == '"')
        {
            inString = true;
            ++i;
            continue;
        }

        if (c.isSpace())
        {
            ++i;
            continue;
        }

        const vsizetype numberEnd = ReadNumberEnd(text, i);
        if (numberEnd > i)
        {
            setFormat(static_cast<int>(i), static_cast<int>(numberEnd - i), m_numberFormat);
            i = numberEnd;
            continue;
        }

        const vsizetype identEnd = ReadIdentifierEnd(text, i);
        if (identEnd > i)
        {
            const QString identifier = text.mid(i, identEnd - i);
            if (const QTextCharFormat *format = FormatForIdentifier(identifier, text, identEnd); format != nullptr)
            {
                setFormat(static_cast<int>(i), static_cast<int>(identEnd - i), *format);
            }
            i = identEnd;
            continue;
        }

        if (i + 1 < size)
        {
            const QString two = text.mid(i, 2);
            if (two == QLatin1String("<=") || two == QLatin1String(">=") || two == QLatin1String("==")
                || two == QLatin1String("!="))
            {
                setFormat(static_cast<int>(i), 2, m_comparisonFormat);
                i += 2;
                continue;
            }
        }

        if (c == '?' || c == ':')
        {
            setFormat(static_cast<int>(i), 1, m_ternaryFormat);
            ++i;
            continue;
        }

        if (c == '<' || c == '>' || c == '!' || c == '&' || c == '|')
        {
            setFormat(static_cast<int>(i), 1, m_comparisonFormat);
            ++i;
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^')
        {
            setFormat(static_cast<int>(i), 1, m_arithmeticFormat);
            ++i;
            continue;
        }

        ++i;
    }

    setCurrentBlockState(inString ? BlockStateInString : BlockStateNormal);
}

//---------------------------------------------------------------------------------------------------------------------
auto VHighlighter::FormatForIdentifier(const QString &identifier, const QString &line, vsizetype end) const
    -> const QTextCharFormat *
{
    if (identifier.startsWith(CustomIncrSign))
    {
        if (m_locals.contains(identifier))
        {
            return &m_localFormat;
        }

        return &m_patternFormat;
    }

    if ((identifier.startsWith(CustomMSign) && identifier.size() > 1)
        || StartsWithAny(identifier, m_measurementPrefixes))
    {
        return &m_measurementFormat;
    }

    if (m_exactGeometry.contains(identifier) || StartsWithAny(identifier, m_geometryPrefixes))
    {
        return &m_geometryFormat;
    }

    if (m_functions.contains(identifier))
    {
        vsizetype j = end;
        const vsizetype size = line.size();
        while (j < size && line.at(j).isSpace())
        {
            ++j;
        }
        if (j < size && line.at(j) == '(')
        {
            return &m_functionFormat;
        }
    }

    if (identifier == QLatin1String("_pi") || identifier == QLatin1String("_e"))
    {
        return &m_numberFormat;
    }

    return nullptr;
}
