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



#ifndef UT_MKEYBOARDSETTINGS_H
#define UT_MKEYBOARDSETTINGS_H

#include <QObject>
#include <QtTest/QTest>

class Ut_MKeyboardSettings : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testNoCrash();
    void testAvailableKeyboards();
    void testSelectedKeyboards();
    void testSetSelectedKeyboards();
    void testErrorCorrection();

};

#endif

