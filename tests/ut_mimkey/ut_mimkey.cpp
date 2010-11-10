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

#include <MApplication>
#include <MTheme>

#include <QSignalSpy>
#include <QDebug>

Q_DECLARE_METATYPE(QList<Ut_MImKey::DirectionPair>)
Q_DECLARE_METATYPE(Ut_MImKey::KeyList)
Q_DECLARE_METATYPE(QList<Ut_MImKey::KeyTriple>)
Q_DECLARE_METATYPE(QList<int>)

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
                                (char *) "-local-theme" };

    disableQtPlugins();
    app = new MApplication(argc, app_name);

    style = new MImAbstractKeyAreaStyleContainer;
    style->initialize("", "", 0);

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
    subject = new MImKey(*dataKey, *style, *parent);
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
    key->bindings[MImKeyModel::NoShift] = binding;

    MImAbstractKey *subject = new MImKey(*key, *style, *parent);

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
    QTest::addColumn< QList<KeyTriple> >("keyControlSequence");
    QTest::addColumn< QList<int> >("expectedActiveKeys");

    QTest::newRow("single key")
        << (KeyList() << createKey())
        << (QList<KeyTriple>() << KeyTriple(0, MImAbstractKey::Pressed, 0))
        << (QList<int>() << 0);

    QTest::newRow("two keys")
        << (KeyList() << createKey() << createKey())
        << (QList<KeyTriple>() << KeyTriple(0, MImAbstractKey::Pressed, 0)
                               << KeyTriple(1, MImAbstractKey::Selected, 1)
                               << KeyTriple(0, MImAbstractKey::Normal, 1))
        << (QList<int>() << 1);

    QTest::newRow("last active key")
        << (KeyList() << createKey() << createKey() << createKey())
        << (QList<KeyTriple>() << KeyTriple(0, MImAbstractKey::Pressed, 0)
                               << KeyTriple(1, MImAbstractKey::Selected, 1)
                               << KeyTriple(2, MImAbstractKey::Normal, 1)
                               << KeyTriple(1, MImAbstractKey::Normal, 0)
                               << KeyTriple(2, MImAbstractKey::Pressed, 2)
                               << KeyTriple(0, MImAbstractKey::Normal, 2)
                               << KeyTriple(2, MImAbstractKey::Normal, -1))
        << (QList<int>());
}

void Ut_MImKey::testActiveKeys()
{
    QFETCH(KeyList, availableKeys);
    QFETCH(QList<KeyTriple>, keyControlSequence);
    QFETCH(QList<int>, expectedActiveKeys);
    const MImAbstractKey *const noKey = 0;

    foreach (const KeyTriple &triple, keyControlSequence) {
        availableKeys.at(triple.index)->setDownState(isActiveKeyState(triple.state));
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
    model->bindings[MImKeyModel::NoShift] = b;
    MImKey *shift = new MImKey(*model, *style, *parent);
    shift->setDownState(true);
    keys << shift;

    SpecialKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);
    QCOMPARE(finder.shiftKey(), shift);
    QCOMPARE(finder.visits(), keys.count());
}

MImKey *Ut_MImKey::createKey(bool state)
{
    MImKey *key = new MImKey(*dataKey, *style, *parent);
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

    key->bindings[MImKeyModel::NoShift] = binding1;
    key->bindings[MImKeyModel::Shift] = binding2;

    return key;
}

QTEST_APPLESS_MAIN(Ut_MImKey);
