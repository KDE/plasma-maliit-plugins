/* * This file is part of m-keyboard *
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

#ifndef UT_BUTTONBAR_H
#define UT_BUTTONBAR_H

#include <QtTest/QtTest>
#include <QObject>

class MApplication;
class MButton;
class ButtonBar;

class Ut_ButtonBar : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    ButtonBar *subject;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testInsert_data();
    void testInsert();

    void testRemove_data();
    void testRemove();

    void testLayoutContent_data();
    void testLayoutContent();

    void testIndexOf();
};

#endif // UT_BUTTONBAR_H
