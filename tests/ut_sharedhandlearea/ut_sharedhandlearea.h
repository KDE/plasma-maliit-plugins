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



#ifndef UT_SHAREDHANDLEAREA
#define UT_SHAREDHANDLEAREA

#include <QtTest/QTest>
#include <QObject>
#include <QPointer>
#include <QGraphicsWidget>
#include <mimtoolbar.h>

class MApplication;
class SharedHandleArea;

class Ut_SharedHandleArea : public QObject
{
    Q_OBJECT

    MApplication *app;
    QPointer<SharedHandleArea> subject;
    QPointer<QGraphicsWidget> parent;
    QPointer<MImToolbar> imToolbar;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testWatching();
};

#endif

