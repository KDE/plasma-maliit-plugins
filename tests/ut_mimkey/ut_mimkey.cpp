/* * This file is part of meego-keyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */



#include "ut_mimkey.h"

#include "mimabstractkeyareastyle.h"
#include "mimkey.h"
#include "mimkeyarea.h"
#include "mimkeymodel.h"
#include "utils.h"

#include <mkeyoverride.h>

#include <MApplication>
#include <MTheme>

#include <QSignalSpy>
#include <QDebug>

#include <memory>

Q_DECLARE_METATYPE(QList<Ut_MImKey::DirectionPair>)
Q_DECLARE_METATYPE(Ut_MImKey::KeyList)
Q_DECLARE_METATYPE(QList<Ut_MImKey::KeyTriple>)
Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(Ut_MImKey::ModelList)

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
}

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

    parent = new QGraphicsWidget;
    dataKey = createKeyModel();
}

void Ut_MImKey::cleanupTestCase()
{
    delete style;
    delete dataKey;
    delete app;
    app = 0;
    delete parent;
}

void Ut_MImKey::init()
{
    subject = new MImKey(*dataKey, *style, *parent, stylingCache);
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

    MImAbstractKey *subject = new MImKey(*key, *style, *parent, stylingCache);

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
    const MImAbstractKey *const noKey = 0;

    foreach (const KeyTriple &triple, keyControlSequence) {
        MImAbstractKey *key = availableKeys.at(triple.index);

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
    MImKey *shift = new MImKey(*model, *style, *parent, stylingCache);
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
    key->updateGeometryCache();
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
    key->updateGeometryCache();
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

MImKey *Ut_MImKey::createKey(bool state)
{
    MImKey *key = new MImKey(*dataKey, *style, *parent, stylingCache);
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
    MImKey *key = new MImKey(*model, *style, *parent, stylingCache);
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
