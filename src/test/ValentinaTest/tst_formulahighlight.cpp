/************************************************************************
 **
 **  @file   tst_formulahighlight.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 10, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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

#include "tst_formulahighlight.h"

#include "../vmisc/theme/vtheme.h"
#include "../vwidgets/vhighlighter.h"
#include "../vwidgets/vplaintextedit.h"

#include <QFont>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextLayout>
#include <QtTest>

namespace
{
struct FormulaColors
{
    QColor local{};
    QColor pattern{};
    QColor measurement{};
    QColor geometry{};
    QColor function{};
    QColor number{};
    QColor ternary{};
    QColor arithmetic{};
};

//---------------------------------------------------------------------------------------------------------------------
auto Palette() -> FormulaColors
{
    if (VTheme::IsInDarkTheme())
    {
        return {QColor(0x8e, 0xc0, 0x7c),
                QColor(0x83, 0xa5, 0x98),
                QColor(0xfa, 0xbd, 0x2f),
                QColor(0xa8, 0x99, 0x84),
                QColor(0xb8, 0xbb, 0x26),
                QColor(0xd3, 0x86, 0x9b),
                QColor(0xfe, 0x80, 0x19),
                QColor(0xd6, 0x5d, 0x0e)};
    }

    return {QColor(0x42, 0x7b, 0x58),
            QColor(0x07, 0x66, 0x78),
            QColor(0xb5, 0x76, 0x14),
            QColor(0x7c, 0x6f, 0x64),
            QColor(0x79, 0x74, 0x0e),
            QColor(0x8f, 0x3f, 0x71),
            QColor(0xaf, 0x3a, 0x03),
            QColor(0x9d, 0x00, 0x06)};
}

//---------------------------------------------------------------------------------------------------------------------
auto ScreenshotFormula() -> QString
{
    return QStringLiteral("#back = Line_A72_A69;\n"
                          "#side = Line_A54_A76+Line_A51_A75+Line_A51_A54;\n"
                          "#front = Line_A107_A109+Line_A109_A105+Line_A105_A48+Line_A48_A77;\n"
                          "#hipCalculated = (#back+#side+#front);\n"
                          "#hipDesired = ((M_hipSeatGirth+#fitHip)/2);\n"
                          "#hipDesired - #hipCalculated <= 0 ? (0) : (#hipDesired - #hipCalculated)");
}

//---------------------------------------------------------------------------------------------------------------------
auto FormatAt(const QTextDocument &doc, int position) -> QTextCharFormat
{
    // QSyntaxHighlighter writes into QTextLayout::formats(). QTextBlock::textFormats() only returns the
    // document's own character formats, which stay at the default brush.
    const QTextBlock block = doc.findBlock(position);
    const QTextLayout *layout = block.layout();
    if (layout == nullptr)
    {
        return {};
    }

    const int offset = position - static_cast<int>(block.position());
    QTextCharFormat format;
    // Qt 5 returns QVector. Qt 6 returns QList. The two do not convert.
    const auto ranges = layout->formats();
    for (const QTextLayout::FormatRange &range : ranges)
    {
        if (offset >= range.start && offset < range.start + range.length)
        {
            format.merge(range.format);
        }
    }

    return format;
}

//---------------------------------------------------------------------------------------------------------------------
auto ExpectColor(
    const QTextDocument &doc, const QString &formula, const QString &token, const QColor &color, int from = 0) -> bool
{
    const int pos = static_cast<int>(formula.indexOf(token, from));
    if (pos < 0)
    {
        qWarning() << "missing token" << token;
        return false;
    }

    const QColor actual = FormatAt(doc, pos).foreground().color();
    if (actual != color)
    {
        qWarning() << token << "at" << pos << "is" << actual.name() << "expected" << color.name();
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto CheckScreenshot(const QString &formula) -> bool
{
    // QPlainTextEdit, which is what formula fields use. A bare QTextDocument with Qt's rich-text
    // layout drops syntax formats applied during setPlainText.
    VPlainTextEdit edit;
    edit.setPlainText(formula);
    const QTextDocument &doc = *edit.document();

    const FormulaColors colors = Palette();
    const int declared = static_cast<int>(formula.indexOf(QStringLiteral("#back")));
    if (declared < 0)
    {
        qWarning() << "missing #back";
        return false;
    }

    if (not ExpectColor(doc, formula, QStringLiteral("#back"), colors.local)
        || not ExpectColor(doc, formula, QStringLiteral("#back"), colors.local, declared + 1)
        || not ExpectColor(doc, formula, QStringLiteral("#hipDesired"), colors.local)
        || not ExpectColor(doc, formula, QStringLiteral("#fitHip"), colors.pattern)
        || not ExpectColor(doc, formula, QStringLiteral("M_hipSeatGirth"), colors.measurement)
        || not ExpectColor(doc, formula, QStringLiteral("Line_A72_A69"), colors.geometry)
        || not ExpectColor(doc, formula, QStringLiteral("<="), colors.ternary)
        || not ExpectColor(doc, formula, QStringLiteral("?"), colors.ternary)
        || not ExpectColor(doc, formula, QStringLiteral(":"), colors.ternary)
        || not ExpectColor(doc,
                           formula,
                           QStringLiteral("0"),
                           colors.number,
                           static_cast<int>(formula.indexOf(QStringLiteral("<= 0"))) + 3)
        || not ExpectColor(doc, formula, QStringLiteral("+"), colors.arithmetic)
        || not ExpectColor(doc, formula, QStringLiteral("-"), colors.arithmetic))
    {
        return false;
    }

    const int question = static_cast<int>(formula.indexOf('?'));
    const int colon = static_cast<int>(formula.indexOf(':'));
    const int compare = static_cast<int>(formula.indexOf(QStringLiteral("<=")));
    if (FormatAt(doc, question).fontWeight() != static_cast<int>(QFont::Bold)
        || FormatAt(doc, colon).fontWeight() != static_cast<int>(QFont::Bold))
    {
        qWarning() << "ternary markers are not bold";
        return false;
    }

    if (FormatAt(doc, compare).fontWeight() == static_cast<int>(QFont::Bold))
    {
        qWarning() << "comparison is bold";
        return false;
    }

    return true;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_FormulaHighlight::TST_FormulaHighlight(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaHighlight::ColorsScreenshotFormula() const
{
    const QString wrapped = ScreenshotFormula();
    QVERIFY(CheckScreenshot(wrapped));
    QVERIFY(CheckScreenshot(QString(wrapped).replace('\n', QString())));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaHighlight::LocalOnLaterLineTracksDeclaration()
{
    VPlainTextEdit edit;
    edit.setPlainText(QStringLiteral("#back = 1;\n#back + #fitHip"));
    const QTextDocument &doc = *edit.document();

    const FormulaColors colors = Palette();
    const QString text = doc.toPlainText();
    QVERIFY(ExpectColor(doc, text, QStringLiteral("#back"), colors.local, static_cast<int>(text.indexOf('\n'))));
    QVERIFY(ExpectColor(doc, text, QStringLiteral("#fitHip"), colors.pattern));

    QTextCursor cursor(edit.document());
    cursor.setPosition(0);
    cursor.setPosition(static_cast<int>(QStringLiteral("#back").size()), QTextCursor::KeepAnchor);
    cursor.insertText(QStringLiteral("#side"));

    const QString updated = doc.toPlainText();
    QVERIFY(updated.startsWith(QStringLiteral("#side = 1;")));
    QVERIFY(ExpectColor(doc, updated, QStringLiteral("#side"), colors.local));
    QVERIFY(ExpectColor(doc, updated, QStringLiteral("#back"), colors.pattern));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaHighlight::DisabledHighlightingLeavesTextPlain()
{
    VPlainTextEdit edit;
    QVERIFY(edit.IsFormulaHighlightingEnabled());
    edit.SetFormulaHighlightingEnabled(false);
    QVERIFY(not edit.IsFormulaHighlightingEnabled());

    const QString text = QStringLiteral("#back = M_hipSeatGirth + #fitHip ? 1 : 0");
    edit.setPlainText(text);

    for (int i = 0; i < text.size(); ++i)
    {
        QVERIFY(FormatAt(*edit.document(), i).foreground().style() == Qt::NoBrush);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaHighlight::SupportingTokens() const
{
    const FormulaColors colors = Palette();

    VPlainTextEdit edit;
    const QTextDocument &doc = *edit.document();

    edit.setPlainText(QStringLiteral("#back == 1"));
    QVERIFY(ExpectColor(doc, doc.toPlainText(), QStringLiteral("#back"), colors.pattern));

    edit.setPlainText(QStringLiteral("#back<=1"));
    QVERIFY(ExpectColor(doc, doc.toPlainText(), QStringLiteral("#back"), colors.pattern));
    QVERIFY(ExpectColor(doc, doc.toPlainText(), QStringLiteral("<="), colors.ternary));

    edit.setPlainText(QStringLiteral("#back != #fitHip"));
    QVERIFY(ExpectColor(doc, doc.toPlainText(), QStringLiteral("#back"), colors.pattern));
    QVERIFY(ExpectColor(doc, doc.toPlainText(), QStringLiteral("#fitHip"), colors.pattern));

    edit.setPlainText(QStringLiteral("#a = #b = 1"));
    QVERIFY(ExpectColor(doc, doc.toPlainText(), QStringLiteral("#a"), colors.local));
    QVERIFY(ExpectColor(doc, doc.toPlainText(), QStringLiteral("#b"), colors.local));

    edit.setPlainText(QStringLiteral("sinD(@hip) + _pi"));
    QVERIFY(ExpectColor(doc, doc.toPlainText(), QStringLiteral("sinD"), colors.function));
    QVERIFY(ExpectColor(doc, doc.toPlainText(), QStringLiteral("@hip"), colors.measurement));
    QVERIFY(ExpectColor(doc, doc.toPlainText(), QStringLiteral("_pi"), colors.number));

    edit.setPlainText(QStringLiteral("sinD"));
    QVERIFY(FormatAt(doc, 0).foreground().style() == Qt::NoBrush);

    edit.setPlainText(QStringLiteral("CurrentLength + CurrentLengthExtra"));
    QVERIFY(ExpectColor(doc, doc.toPlainText(), QStringLiteral("CurrentLength"), colors.geometry));
    const int extra = static_cast<int>(doc.toPlainText().indexOf(QStringLiteral("CurrentLengthExtra")));
    QVERIFY(FormatAt(doc, extra).foreground().style() == Qt::NoBrush);

    const QString quoted = QStringLiteral("warning(\"#back = 1\", #back)");
    edit.setPlainText(quoted);
    const int inside = static_cast<int>(quoted.indexOf(QStringLiteral("#back")));
    const int outside = static_cast<int>(quoted.lastIndexOf(QStringLiteral("#back")));
    QVERIFY(inside >= 0 && outside > inside);
    QVERIFY(FormatAt(doc, inside).foreground().style() == Qt::NoBrush);
    QVERIFY(ExpectColor(doc, quoted, QStringLiteral("#back"), colors.pattern, outside));
    QVERIFY(ExpectColor(doc, quoted, QStringLiteral("warning"), colors.function));

    const QString multiline = QStringLiteral("warning(\"M_hip\n#fitHip\", 1)");
    edit.setPlainText(multiline);
    const int hidden = static_cast<int>(multiline.indexOf(QStringLiteral("#fitHip")));
    QVERIFY(FormatAt(doc, hidden).foreground().style() == Qt::NoBrush);
    QVERIFY(ExpectColor(doc, multiline, QStringLiteral("1"), colors.number));

    edit.setPlainText(QStringLiteral("#fit"));
    QVERIFY(ExpectColor(doc, doc.toPlainText(), QStringLiteral("#fit"), colors.pattern));
    edit.setPlainText(QStringLiteral("#fit = 1"));
    QVERIFY(ExpectColor(doc, doc.toPlainText(), QStringLiteral("#fit"), colors.local));
}
