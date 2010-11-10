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



#ifndef UT_MIMKEY_H
#define UT_MIMKEY_H

#include <mimabstractkey.h>

#include <QtTest/QTest>
#include <QObject>
#include <QList>

class MApplication;
class MImKeyModel;
class MImKey;
class MImAbstractKeyAreaStyleContainer;
class QGraphicsItem;
class KeyboardData;

class Ut_MImKey: public QObject
{
    Q_OBJECT

public:
    enum Direction {
        Down,
        Up
    };

    struct KeyTriple {
        int index;
        MImAbstractKey::ButtonState state;
        int lastActiveIndex;

        // Needed for metatype registration:
        KeyTriple()
            : index(-1)
            , state(MImAbstractKey::Normal)
            , lastActiveIndex(-1)
        {}

        KeyTriple(int newIndex,
                  MImAbstractKey::ButtonState newState,
                  int newLastActiveIndex)
            : index(newIndex)
            , state(newState)
            , lastActiveIndex(newLastActiveIndex)
        {}

    };

    typedef QPair<Direction, bool> DirectionPair;
    typedef QList<MImAbstractKey *> KeyList;

private:

    MApplication *app;
    MImAbstractKey *subject;
    QGraphicsItem *parent;
    MImKeyModel *dataKey;
    MImAbstractKeyAreaStyleContainer *style;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testSetModifier_data();
    void testSetModifier();
    void testKey();
    void testBinding();
    void testIsDead();

    void testTouchPointCount_data();
    void testTouchPointCount();
    void testResetTouchPointCount();

    void testActiveKeys_data();
    void testActiveKeys();
    void testResetActiveKeys();
    void testVisitActiveKeys();

private:
    MImKey *createKey(bool state = false);
    MImKeyModel *createKeyModel();
};

Q_DECLARE_METATYPE(Ut_MImKey::DirectionPair)
Q_DECLARE_METATYPE(Ut_MImKey::KeyTriple)
Q_DECLARE_METATYPE(MImAbstractKey::ButtonState)

#endif

