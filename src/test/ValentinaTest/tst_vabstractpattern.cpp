/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 7, 2026
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

#include "tst_vabstractpattern.h"

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vtoolrecord.h"

#include <QtTest>

namespace
{
// Minimal concrete pattern document: VAbstractPattern is abstract, but ChangeNamePP() and the history it maintains
// need none of the pure virtuals.
class StubPattern : public VAbstractPattern
{
public:
    StubPattern() = default;

    using VAbstractPattern::SetActivPP;

    void CreateEmptyFile() override {}
    void IncrementReferens(quint32 id) const override { Q_UNUSED(id) }
    void DecrementReferens(quint32 id) const override { Q_UNUSED(id) }
    auto GenerateLabel(const LabelType &type, const QString &reservedName) const -> QString override
    {
        Q_UNUSED(type)
        Q_UNUSED(reservedName)
        return {};
    }
    auto GenerateSuffix() const -> QString override { return {}; }
    void UpdateToolData(const quint32 &id, VContainer *data) override
    {
        Q_UNUSED(id)
        Q_UNUSED(data)
    }
    void LiteParseTree(const Document &parse) override { Q_UNUSED(parse) }
};
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_VAbstractPattern::TST_VAbstractPattern(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChangeNamePPKeepsLocalHistory renaming a pattern piece must carry its history records over to the new name.
 *
 * Losing them left the History dialog with an empty table while the global history was not empty, which crashed the
 * dialog on open.
 */
void TST_VAbstractPattern::ChangeNamePPKeepsLocalHistory()
{
    StubPattern doc;
    QVERIFY(doc.setContent(QByteArray(R"(<pattern><draw name="Block A"/><draw name="Block B"/></pattern>)")));

    doc.SetActivPP(QStringLiteral("Block A"));

    QVector<VToolRecord> *history = doc.getHistory();
    history->append(VToolRecord(1, Tool::BasePoint, QStringLiteral("Block A")));
    history->append(VToolRecord(2, Tool::EndLine, QStringLiteral("Block A")));
    history->append(VToolRecord(3, Tool::BasePoint, QStringLiteral("Block B")));

    QCOMPARE(doc.getLocalHistory().size(), 2);

    QVERIFY(doc.ChangeNamePP(QStringLiteral("Block A"), QStringLiteral("Block A renamed")));

    QCOMPARE(doc.GetNameActivPP(), QStringLiteral("Block A renamed"));
    QCOMPARE(doc.getLocalHistory().size(), 2);
    QCOMPARE(doc.getLocalHistory().at(0).getId(), static_cast<quint32>(1));
    QCOMPARE(doc.getLocalHistory().at(1).getId(), static_cast<quint32>(2));

    // The other pattern piece must be untouched.
    doc.SetActivPP(QStringLiteral("Block B"));
    QCOMPARE(doc.getLocalHistory().size(), 1);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MaxRecordedIdCountsOrphanedNodes a full parse must not reissue an id that only survives on an
 * orphaned node.
 *
 * When a node's source object is deleted, the node element itself is left behind in the document (its
 * idObject no longer resolves), and parsing it just skips recreating a live tool for it. MaxRecordedId() feeds
 * the id generator after a full-parse reset, so it must count that element's id too, or a freshly minted id
 * can collide with it -- exactly the crash this test is standing in for.
 */
void TST_VAbstractPattern::MaxRecordedIdCountsOrphanedNodes()
{
    StubPattern doc;
    QVERIFY(doc.setContent(QByteArray(R"(
        <pattern>
            <draw name="Block A">
                <calculation>
                    <point id="5" type="single"/>
                    <line id="10" firstPoint="5" secondPoint="5"/>
                </calculation>
                <modeling>
                    <spline id="161" idObject="78" inUse="false" type="modelingSpline"/>
                </modeling>
            </draw>
        </pattern>)")));

    doc.RefreshElementIdCache();

    QCOMPARE(doc.MaxRecordedId(), static_cast<quint32>(161));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPattern::MaxRecordedIdOnEmptyDocumentIsZero()
{
    StubPattern doc;
    QVERIFY(doc.setContent(QByteArray(R"(<pattern><draw name="Block A"/></pattern>)")));

    doc.RefreshElementIdCache();

    QCOMPARE(doc.MaxRecordedId(), static_cast<quint32>(NULL_ID));
}
