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



#ifndef UT_KEYBUTTON_H
#define UT_KEYBUTTON_H

#include <mimabstractkey.h>

#include <QtTest/QTest>
#include <QObject>
#include <QList>

class MApplication;
class MImKeyModel;
class MImAbstractKeyAreaStyleContainer;
class QGraphicsItem;
class KeyboardData;

class Ut_KeyButton: public QObject
{
    Q_OBJECT

public:
    enum Direction {
        Down,
        Up
    };

    typedef QPair<Direction, bool> DirectionPair;

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

private:
    MImKeyModel *createDataKey();
};

Q_DECLARE_METATYPE(Ut_KeyButton::DirectionPair)
Q_DECLARE_METATYPE(MImAbstractKey::ButtonState)

#endif

