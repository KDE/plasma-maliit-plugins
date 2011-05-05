/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list 
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list 
 * of conditions and the following disclaimer in the documentation and/or other materials 
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be 
 * used to endorse or promote products derived from this software without specific 
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */



#include "ut_mimkey.h"

#include "mimabstractkeyareastyle.h"
#include "mimkey.h"
#include "mimkeyarea.h"
#include "mimkeymodel.h"
#include "mimfontpool.h"
#include "utils.h"

#include <mkeyoverride.h>

#include <MApplication>
#include <MTheme>

#include <QSignalSpy>
#include <QDebug>

#include <memory>

namespace {

    class ActiveKeyFinder
            : public MImAbstractKeyVisitor
    {
    public:
        bool found;
        MImAbstractKey *findMe;
        int visits;

        explicit ActiveKeyFinder(MImAbstractKey *newFindMe = 0)
            : found(false)
            , findMe(newFindMe)
            , visits(0)
        {}

        bool operator()(MImAbstractKey *key)
        {
            ++visits;

            found = (found || (key == findMe));
            return found;
        }
    };

    bool isActiveKeyState(MImAbstractKey::ButtonState state)
    {
        return ((state == MImAbstractKey::Pressed)
                || (state == MImAbstractKey::Selected));
    }

    enum IconInfoFlag
    {
        None = 0x0,
        NormalIcon = 0x1,
        CompactIcon = 0x2,
        SelectedIcon = 0x4,
        HighlightedIcon = 0x8
    };
}

Q_DECLARE_METATYPE(QList<Ut_MImKey::DirectionPair>)
Q_DECLARE_METATYPE(Ut_MImKey::KeyList)
Q_DECLARE_METATYPE(QList<Ut_MImKey::KeyTriple>)
Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(Ut_MImKey::ModelList)

Q_DECLARE_FLAGS(IconInfoFlags, IconInfoFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(IconInfoFlags)
Q_DECLARE_METATYPE(IconInfoFlag)
Q_DECLARE_METATYPE(IconInfoFlags)

void Ut_MImKey::initTestCase()
{
    qRegisterMetaType< QList<DirectionPair> >("QList<DirectionPair>");
    qRegisterMetaType<KeyList>("KeyList");
    qRegisterMetaType< QList<KeyTriple> >("QList<KeyTriple>");

    static int argc = 2;
    static char *app_name[] = { (char*) "ut_mimkey",
                                (char *) "-software" };

    disableQtPlugins();
    app = new MApplication(argc, app_name);

    style = new MImAbstractKeyAreaStyleContainer;
    style->initialize("", "", 0);

    stylingCache = QSharedPointer<MImKey::StylingCache>(new MImKey::StylingCache);
    stylingCache->primary   = QFontMetrics((*style)->font());
    stylingCache->secondary = QFontMetrics((*style)->secondaryFont());

    fontPool = new MImFontPool(true);
    fontPool->setDefaultFont((*style)->font());

    parent = new QGraphicsWidget;
    dataKey = createKeyModel();
}

void Ut_MImKey::cleanupTestCase()
{
    delete fontPool;
    delete style;
    delete dataKey;
    delete app;
    app = 0;
    delete parent;
}

void Ut_MImKey::init()
{
    subject = new MImKey(*dataKey, *style, *parent, stylingCache, *fontPool);
}

void Ut_MImKey::cleanup()
{
    MImAbstractKey::resetActiveKeys();
    delete subject;
    subject = 0;
}

void Ut_MImKey::testSetModifier_data()
{
    QTest::addColumn<bool>("shift");
    QTest::addColumn<QChar>("accent");
    QTest::addColumn<QString>("expectedLabel");

    QChar grave(L'`');
    QChar aigu(L'´');
    QChar circonflexe(L'^');

    QTest::newRow("no shift, no accent")            << false << QChar() << "a";
    QTest::newRow("no shift, l'accent grave")       << false << grave << QString(L'à');
    QTest::newRow("no shift, l'accent aigu")        << false << aigu << QString(L'á');
    QTest::newRow("no shift, l'accent circonflexe") << false << circonflexe << QString(L'â');
    QTest::newRow("shift, no accent")               << true  << QChar() << "A";
    QTest::newRow("shift, l'accent grave")          << true  << grave << QString(L'À');
}

void Ut_MImKey::testSetModifier()
{
    QFETCH(bool, shift);
    QFETCH(QChar, accent);
    QFETCH(QString, expectedLabel);

    subject->setModifiers(shift, accent);
    QCOMPARE(subject->label(), expectedLabel);
}

void Ut_MImKey::testKey()
{
    QCOMPARE(&subject->model(), dataKey);
}

void Ut_MImKey::testBinding()
{
    bool shift = false;
    subject->setModifiers(shift);
    QCOMPARE(&subject->binding(), dataKey->binding(shift));

    shift = true;
    subject->setModifiers(shift);
    QCOMPARE(&subject->binding(), dataKey->binding(shift));
}

void Ut_MImKey::testIsDead()
{
    MImKeyModel *key = new MImKeyModel;
    MImKeyBinding *binding = new MImKeyBinding;
    key->setBinding(*binding, false);

    MImAbstractKey *subject = new MImKey(*key, *style, *parent, stylingCache, *fontPool);

    for (int i = 0; i < 2; ++i) {
        bool isDead = (i != 0);
        binding->dead = isDead;
        QCOMPARE(subject->isDeadKey(), isDead);
    }

    delete subject;
    delete key;
}

void Ut_MImKey::testTouchPointCount_data()
{
    QTest::addColumn<int>("initialCount");
    QTest::addColumn< QList<DirectionPair> >("countDirectionList");
    QTest::addColumn<int>("expectedCount");
    QTest::addColumn<MImAbstractKey::ButtonState>("expectedButtonState");

    QTest::newRow("increase and press button")
        << 0 << (QList<DirectionPair>() << DirectionPair(Up, true))
        << 1 << MImAbstractKey::Pressed;

    QTest::newRow("decrease and release button")
        << 1 << (QList<DirectionPair>() << DirectionPair(Down, true))
        << 0 << MImAbstractKey::Normal;

    QTest::newRow("try to take more than possible")
        << 0 << (QList<DirectionPair>() << DirectionPair(Up, true)
                                        << DirectionPair(Down, true)
                                        << DirectionPair(Down, false))
        << 0 << MImAbstractKey::Normal;

    QTest::newRow("try to take more than possible, again")
        << 0 << (QList<DirectionPair>() << DirectionPair(Up, true)
                                        << DirectionPair(Down, true)
                                        << DirectionPair(Down, false)
                                        << DirectionPair(Up, true))
        << 1 << MImAbstractKey::Pressed;

    QTest::newRow("go to the limit")
        << MImKey::touchPointLimit() << (QList<DirectionPair>() << DirectionPair(Up, false))
        << MImKey::touchPointLimit() << MImAbstractKey::Pressed;

    QTest::newRow("go to the limit, again")
        << MImKey::touchPointLimit() << (QList<DirectionPair>() << DirectionPair(Up, false)
                                         << DirectionPair(Down, true)
                                         << DirectionPair(Down, true))
        << MImKey::touchPointLimit() - 2 << MImAbstractKey::Pressed;
}

void Ut_MImKey::testTouchPointCount()
{
    QFETCH(int, initialCount);
    QFETCH(QList<DirectionPair>, countDirectionList);
    QFETCH(int, expectedCount);
    QFETCH(MImAbstractKey::ButtonState, expectedButtonState);

    for (int idx = 0; idx < initialCount; ++idx) {
        subject->increaseTouchPointCount();
    }

    QCOMPARE(subject->touchPointCount(), initialCount);

    foreach(const DirectionPair &pair, countDirectionList) {
        switch (pair.first) {
        case Up:
            QCOMPARE(subject->increaseTouchPointCount(), pair.second);
            break;

        case Down:
            QCOMPARE(subject->decreaseTouchPointCount(), pair.second);
            break;
        }
    }

    QCOMPARE(subject->touchPointCount(), expectedCount);
    QCOMPARE(subject->state(), expectedButtonState);
}

void Ut_MImKey::testResetTouchPointCount()
{
    QCOMPARE(subject->touchPointCount(), 0);

    for (int idx = 0; idx < 3; ++idx) {
        subject->increaseTouchPointCount();
    }

    QCOMPARE(subject->touchPointCount(), 3);

    subject->resetTouchPointCount();
    QCOMPARE(subject->touchPointCount(), 0);
}

void Ut_MImKey::testActiveKeys_data()
{
    QTest::addColumn<KeyList>("availableKeys");
    QTest::addColumn<QList<KeyTriple> >("keyControlSequence");
    QTest::addColumn<QList<int> >("expectedActiveKeys");
    QTest::addColumn<ModelList>("keyModels");
    QTest::addColumn<bool>("switchingOrder");

    QTest::newRow("single key")
        << (KeyList() << createKey())
        << (QList<KeyTriple>() << KeyTriple(0, MImAbstractKey::Pressed, 0))
        << (QList<int>() << 0)
        << (ModelList())
        << true;

    QTest::newRow("two keys")
        << (KeyList() << createKey() << createKey())
        << (QList<KeyTriple>() << KeyTriple(0, MImAbstractKey::Pressed, 0)
                               << KeyTriple(1, MImAbstractKey::Selected, 0)
                               << KeyTriple(0, MImAbstractKey::Normal, 1))
        << (QList<int>() << 1)
        << (ModelList())
        << true;

    QTest::newRow("last active key")
        << (KeyList() << createKey() << createKey() << createKey())
        << (QList<KeyTriple>() << KeyTriple(0, MImAbstractKey::Pressed, 0)
                               << KeyTriple(1, MImAbstractKey::Pressed, 1)
                               << KeyTriple(1, MImAbstractKey::Selected, 0)
                               << KeyTriple(2, MImAbstractKey::Normal, 0)
                               << KeyTriple(1, MImAbstractKey::Normal, 0)
                               << KeyTriple(2, MImAbstractKey::Pressed, 2)
                               << KeyTriple(0, MImAbstractKey::Normal, 2)
                               << KeyTriple(2, MImAbstractKey::Normal, -1))
        << (QList<int>())
        << (ModelList())
        << true;

    QTest::newRow("last active key2")
        << (KeyList() << createKey() << createKey() << createKey())
        << (QList<KeyTriple>() << KeyTriple(0, MImAbstractKey::Pressed, 0)
                               << KeyTriple(1, MImAbstractKey::Pressed, 1)
                               << KeyTriple(1, MImAbstractKey::Selected, 1)
                               << KeyTriple(2, MImAbstractKey::Normal, 1)
                               << KeyTriple(1, MImAbstractKey::Normal, 0)
                               << KeyTriple(2, MImAbstractKey::Pressed, 2)
                               << KeyTriple(0, MImAbstractKey::Normal, 2)
                               << KeyTriple(2, MImAbstractKey::Normal, -1))
        << (QList<int>())
        << (ModelList())
        << false;

    MImKeyModel *model = createDeadKeyModel("^");

    QTest::newRow("normal+dead key")
        << (KeyList() << createKey() << createDeadKey(model))
        << (QList<KeyTriple>() << KeyTriple(1, MImAbstractKey::Selected, 1)
                               << KeyTriple(0, MImAbstractKey::Pressed, 0)
                               << KeyTriple(1, MImAbstractKey::Pressed, 1)
                               << KeyTriple(1, MImAbstractKey::Normal, 0))
        << (QList<int>() << 0)
        << (ModelList() << model)
        << true;
}

void Ut_MImKey::testActiveKeys()
{
    QFETCH(KeyList, availableKeys);
    QFETCH(QList<KeyTriple>, keyControlSequence);
    QFETCH(QList<int>, expectedActiveKeys);
    QFETCH(ModelList, keyModels);
    QFETCH(bool, switchingOrder);
    const MImKey *const noKey = 0;

    foreach (const KeyTriple &triple, keyControlSequence) {
        MImKey *key = availableKeys.at(triple.index);

        switch (triple.state) {
        case MImAbstractKey::Normal:
            key->setSelected(false);
            key->setDownState(false);
            break;
        case MImAbstractKey::Pressed:
            key->setSelected(false);
            key->setDownState(true);
            break;
        case MImAbstractKey::Selected:
            if (switchingOrder) {
                key->setDownState(false);
                key->setSelected(true);
            } else {
                key->setSelected(true);
                key->setDownState(false);
            }
            break;
        default:
            break;
        }

        if (triple.lastActiveIndex > -1) {
            QCOMPARE(MImAbstractKey::lastActiveKey(),
                     availableKeys.at(triple.lastActiveIndex));
        } else {
            QCOMPARE(MImAbstractKey::lastActiveKey(), noKey);
        }
    }

    foreach (int idx, expectedActiveKeys) {
        ActiveKeyFinder finder(availableKeys.at(idx));
        MImAbstractKey::visitActiveKeys(&finder);

        QVERIFY(finder.found);
    }

    // Verify that remaining keys (those not listed in expectedActiveKeys)
    // are not in MImAbstractKey::activeKeys:
    for (int idx = 0; idx < availableKeys.count(); ++idx) {
        if (!expectedActiveKeys.contains(idx)) {
            ActiveKeyFinder finder(availableKeys.at(idx));
            MImAbstractKey::visitActiveKeys(&finder);

            QVERIFY(not finder.found);
        }
    }

    qDeleteAll(availableKeys);
    qDeleteAll(keyModels);
}

void Ut_MImKey::testResetActiveKeys()
{
    ActiveKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);
    QCOMPARE(finder.visits, 0);

    KeyList keys;
    keys << createKey(true) << createKey(false) << createKey(true);
    MImAbstractKey::visitActiveKeys(&finder);
    QCOMPARE(finder.visits, 2);

    MImAbstractKey::resetActiveKeys();
}

void Ut_MImKey::testVisitActiveKeys()
{
    KeyList keys;
    keys << createKey(true) << createKey(true);

    MImKeyBinding *b = new MImKeyBinding;
    b->keyAction = MImKeyBinding::ActionShift;
    MImKeyModel *model = new MImKeyModel;
    model->setBinding(*b, false);
    MImKey *shift = new MImKey(*model, *style, *parent, stylingCache, *fontPool);
    shift->setDownState(true);
    keys << shift;

    SpecialKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);
    QCOMPARE(finder.shiftKey(), shift);
    QCOMPARE(finder.visits(), keys.count());
}

void Ut_MImKey::testKeyRects()
{
    std::auto_ptr<MImKey> key(createKey());
    QVERIFY(key->buttonRect().isEmpty());
    QVERIFY(key->buttonBoundingRect().isEmpty());

    const QPointF topLeft(50, 50);
    key->setPos(topLeft);
    key->handleGeometryChange();
    QCOMPARE(key->buttonRect().topLeft(), topLeft);
    QCOMPARE(key->buttonBoundingRect().topLeft(), topLeft);

    const qreal width(200);
    key->setWidth(width);
    QCOMPARE(key->buttonRect().width(), width);
    QCOMPARE(key->buttonBoundingRect().width(), width);

    const qreal height(100);
    key->setHeight(height);
    QCOMPARE(key->buttonRect().height(), height);
    QCOMPARE(key->buttonBoundingRect().height(), height);

    const qreal margin0(10);
    const qreal margin1(5);
    const QRectF expectedBoundingRect(topLeft.x(), topLeft.y(),
                                      width + margin0 + margin1,
                                      height + margin1 + margin0);
    const QRectF expectedRect(expectedBoundingRect.adjusted( margin0,  margin1,
                                                            -margin1, -margin0));
    key->setMargins(margin0, margin1, margin1, margin0);
    QCOMPARE(key->buttonRect(), expectedRect);
    QCOMPARE(key->buttonBoundingRect(), expectedBoundingRect);

    key.reset(createKey());
    key->setGeometry(MImKey::Geometry(width, height, margin0, margin1, margin1, margin0));
    key->setPos(topLeft);
    key->handleGeometryChange();
    QCOMPARE(key->buttonRect(), expectedRect);
    QCOMPARE(key->buttonBoundingRect(), expectedBoundingRect);
}

void Ut_MImKey::testGravity()
{
    QVERIFY(!subject->isGravityActive());

    subject->activateGravity();
    QVERIFY(subject->isGravityActive());

    subject->setDownState(true);
    QVERIFY(subject->isGravityActive());

    subject->setDownState(false);
    QVERIFY(!subject->isGravityActive());
}

void Ut_MImKey::testLabelOverride()
{
    QSharedPointer<MKeyOverride> override(new MKeyOverride("keyId"));
    const QString originalLabel = subject->label();
    const QString newLabel("label");

    QVERIFY(newLabel != originalLabel);

    subject->setKeyOverride(override);
    QCOMPARE(subject->label(), originalLabel);

    override->setLabel(newLabel);
    subject->updateOverrideAttributes(MKeyOverride::Label);
    QCOMPARE(subject->label(), newLabel);

    subject->setIgnoreOverriding(true);
    QCOMPARE(subject->label(), originalLabel);

    subject->setIgnoreOverriding(false);
    QCOMPARE(subject->label(), newLabel);

    subject->resetKeyOverride();
    QCOMPARE(subject->label(), originalLabel);
}

void Ut_MImKey::testKeyDisabling()
{
    QSharedPointer<MKeyOverride> override(new MKeyOverride("keyId"));

    QVERIFY(subject->enabled());

    subject->setKeyOverride(override);
    QVERIFY(subject->enabled());

    subject->setDownState(true);
    QCOMPARE(subject->state(), MImAbstractKey::Pressed);

    subject->setDownState(false);
    subject->setSelected(true);
    QCOMPARE(subject->state(), MImAbstractKey::Selected);

    subject->setSelected(false);
    QCOMPARE(subject->state(), MImAbstractKey::Normal);

    override->setEnabled(false);
    subject->updateOverrideAttributes(MKeyOverride::Enabled);
    QVERIFY(!subject->enabled());
    QCOMPARE(subject->state(), MImAbstractKey::Disabled);

    subject->setDownState(true);
    QCOMPARE(subject->state(), MImAbstractKey::Disabled);

    subject->setSelected(true);
    QCOMPARE(subject->state(), MImAbstractKey::Disabled);

    subject->resetKeyOverride();
    QVERIFY(subject->enabled());
    QCOMPARE(subject->state(), MImAbstractKey::Normal);

    subject->setDownState(true);
    QCOMPARE(subject->state(), MImAbstractKey::Pressed);

    subject->setDownState(false);
    subject->setSelected(true);
    QCOMPARE(subject->state(), MImAbstractKey::Selected);

    subject->setSelected(false);
    QCOMPARE(subject->state(), MImAbstractKey::Normal);
}

void Ut_MImKey::testOverrideBinding()
{
    // Make sure default bindings are there.
    bool shift = false;
    subject->setModifiers(shift);
    QCOMPARE(&subject->binding(), dataKey->binding(shift));

    shift = true;
    subject->setModifiers(shift);
    QCOMPARE(&subject->binding(), dataKey->binding(shift));

    // Override bindigs and verify the result:
    MImKeyBinding override("override");
    subject->overrideBinding(&override);

    shift = false;
    subject->setModifiers(shift);
    QCOMPARE(&subject->binding(), &override);
    shift = true;
    subject->setModifiers(shift);
    QCOMPARE(&subject->binding(), &override);

    // Clear the override:
    subject->overrideBinding(0);
    shift = false;
    subject->setModifiers(shift);
    QCOMPARE(&subject->binding(), dataKey->binding(shift));
    shift = true;
    subject->setModifiers(shift);
    QCOMPARE(&subject->binding(), dataKey->binding(shift));
}

void Ut_MImKey::testIconInfo_data()
{
    // Input values
    QTest::addColumn<bool>("isUpperCase");
    QTest::addColumn<bool>("hasNormalIcon"); // Typically true
    QTest::addColumn<QSize>("keySize");
    QTest::addColumn<bool>("hasCompactIcon");

    // The expected result
    QTest::addColumn<QString>("expectedIconType");

    // Essential cases: If the key is so small that the normal icon does not fit AND
    // a compact icon exists, return the compact icon info. Else return the normal icon info.
    QTest::newRow("Lowercase. Normal icon: fits")
            << false << true << QSize(50, 50) << false << "normal";

    QTest::newRow("Lowercase. Normal icon: too tall, Compact icon: exists")
            << false << true << QSize(50, 45) << true << "compact";
    QTest::newRow("Lowercase. Normal icon: too tall, Compact icon: non-existant")
            << false << true << QSize(50, 45) << false << "normal";

    QTest::newRow("Lowercase. Normal icon: too wide, Compact icon: exists")
            << false << true << QSize(45, 50) << true << "compact";
    QTest::newRow("Lowercase. Normal icon: too wide, Compact icon: non-existant")
            << false << true << QSize(45, 50) << false << "normal";

    // Non-critical case: if the normal icon does not exist, try the compact icon
    QTest::newRow("Lowercase. Normal icon: non-existant")
            << false << false << QSize(45, 50) << true << "compact";

    // Same for upper case
    QTest::newRow("Uppercase. Normal icon: fits")
            << true << true << QSize(50, 50) << false << "normal";

    QTest::newRow("Uppercase. Normal icon: too tall, Compact icon: exists")
            << true << true << QSize(50, 45) << true << "compact";
    QTest::newRow("Uppercase. Normal icon: too tall, Compact icon: non-existant")
            << true << true << QSize(50, 45) << false << "normal";

    QTest::newRow("Uppercase. Normal icon: too wide, Compact icon: exists")
            << true << true << QSize(45, 50) << true << "compact";
    QTest::newRow("Uppercase. Normal icon: too wide, Compact icon: non-existant")
            << true << true << QSize(45, 50) << false << "normal";

    QTest::newRow("Uppercase. Normal icon: non-existant")
            << true << false << QSize(45, 50) << true << "compact";

}

void Ut_MImKey::testIconInfo()
{
    QFETCH(bool, isUpperCase);
    QFETCH(QSize, keySize);
    QFETCH(bool, hasNormalIcon);
    QFETCH(bool, hasCompactIcon);
    QFETCH(QString, expectedIconType);

    QSize testKeyMargins(10, 10);
    QPixmap normalIconPixmap(40, 40);
    QPixmap compactIconPixmap(33, 33);
    MImKey::Geometry keyGeometry(keySize.width(), keySize.height(), 0.0, 0.0, 0.0, 0.0);

    // Setup
    MImAbstractKeyAreaStyle *s = const_cast<MImAbstractKeyAreaStyle *>(style->operator->());
    s->setRequiredKeyIconMargins(testKeyMargins);

    subject = new MImKey(*dataKey, *style, *parent, stylingCache, *fontPool);

    subject->lowerCaseIcon.pixmap = hasNormalIcon ? &normalIconPixmap : 0;
    subject->lowerCaseCompactIcon.pixmap = hasCompactIcon ? &compactIconPixmap : 0;
    subject->upperCaseIcon.pixmap = hasNormalIcon ? &normalIconPixmap : 0;
    subject->upperCaseCompactIcon.pixmap = hasCompactIcon ? &compactIconPixmap : 0;

    subject->setModifiers(isUpperCase);

    // Execute test
    subject->setGeometry(keyGeometry);

    if (expectedIconType == "compact") {
        if (isUpperCase) {
            QVERIFY(&subject->iconInfo() == &subject->upperCaseCompactIcon);
        } else {
            QVERIFY(&subject->iconInfo() == &subject->lowerCaseCompactIcon);
        }
    } else if (expectedIconType == "normal") {
        if (isUpperCase) {
            QVERIFY(&subject->iconInfo() == &subject->upperCaseIcon);
        } else {
            QVERIFY(&subject->iconInfo() == &subject->lowerCaseIcon);
        }
    } else {
        Q_ASSERT(false); // Should never happen
    }

    subject->lowerCaseIcon.pixmap = 0;
    subject->lowerCaseCompactIcon.pixmap = 0;
    subject->upperCaseIcon.pixmap = 0;
    subject->upperCaseCompactIcon.pixmap = 0;
}

MImKey *Ut_MImKey::createKey(bool state)
{
    MImKey *key = new MImKey(*dataKey, *style, *parent, stylingCache, *fontPool);
    key->setDownState(state);
    return key;
}

MImKeyModel *Ut_MImKey::createKeyModel()
{
    MImKeyModel *key = new MImKeyModel;

    MImKeyBinding *binding1 = new MImKeyBinding;
    binding1->keyLabel = "a";
    binding1->dead = false;
    binding1->accents = "`´^¨";
    binding1->accented_labels = QString(L'à') + L'á' + L'á' + L'â' + L'ä';
    binding1->keyAction = MImKeyBinding::ActionInsert;

    MImKeyBinding *binding2 = new MImKeyBinding;
    binding2->keyLabel = "A";
    binding2->dead = false;
    binding2->accents = "`´^¨";
    binding2->accented_labels = QString(L'À') + L'Á' + L'Â' + L'Ä';
    binding2->keyAction = MImKeyBinding::ActionInsert;

    key->setBinding(*binding1, false);
    key->setBinding(*binding2, true);

    return key;
}

MImKey *Ut_MImKey::createDeadKey(MImKeyModel *model, bool state)
{
    MImKey *key = new MImKey(*model, *style, *parent, stylingCache, *fontPool);
    key->setDownState(state);
    return key;
}

MImKeyModel *Ut_MImKey::createDeadKeyModel(const QString &label)
{
    MImKeyModel *key = new MImKeyModel;

    MImKeyBinding *binding1 = new MImKeyBinding(label);
    binding1->dead = true;

    key->setBinding(*binding1, false);
    key->setBinding(*binding1, true);

    return key;
}

QTEST_APPLESS_MAIN(Ut_MImKey);
