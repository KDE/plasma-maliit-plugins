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

#ifndef UT_LAYOUTDATA_H
#define UT_LAYOUTDATA_H

#include <QObject>
#include <QtTest/QtTest>

class MApplication;
class MVirtualKeyboardStyleContainer;

class Ut_LayoutData : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MVirtualKeyboardStyleContainer *style;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testConstructFromString();
};

#endif
