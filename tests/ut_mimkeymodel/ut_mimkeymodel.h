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



#ifndef UT_MIMKEYMODEL_H
#define UT_MIMKEYMODEL_H

#include <QtTest/QTest>
#include <QObject>

class MApplication;
class MImKeyModel;

class Ut_MImKeyModel : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MImKeyModel *subject;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testCreate();
    void testAccent();
    void testKeyCode_data();
    void testKeyCode();
};

#endif

