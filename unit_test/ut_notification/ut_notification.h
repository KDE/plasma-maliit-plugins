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



#ifndef UT_NOTIFICATION_H
#define UT_NOTIFICATION_H

#include <QtTest/QTest>
#include <QObject>

class MApplication;
class Notification;
class MVirtualKeyboardStyleContainer;

class Ut_Notification : public QObject
{
    Q_OBJECT

private:
    MApplication *app;
    Notification *subject;
    MVirtualKeyboardStyleContainer *style;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testCreate();
    void testFadeInFadeOut();
    void testCSS();
};

#endif
