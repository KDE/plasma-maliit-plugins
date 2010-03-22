/* * This file is part of dui-keyboard *
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



#include "ut_duikeyboardplugin.h"

#include <QtTest/QTest>
#include <QObject>
#include <QDebug>

void Ut_DuiKeyboardPlugin::initTestCase()
{
    // check that Plugin can be accessed
    m_subject = new DuiKeyboardPlugin();
}


void Ut_DuiKeyboardPlugin::cleanupTestCase()
{
    delete m_subject;
    m_subject = 0;
}


void Ut_DuiKeyboardPlugin::init()
{
}


void Ut_DuiKeyboardPlugin::cleanup()
{
}


void Ut_DuiKeyboardPlugin::testMethods()
{
    // some simple checks that the functions seems to work
    // check name of plugin
    QVERIFY(m_subject->name() == "DuiKeyboard");

    // check possible languages
    QVERIFY(m_subject->languages().count() == 1);

    // only language at the moment is 'en' for this plugin
    QVERIFY(m_subject->languages().indexOf("en") != -1);
}


QTEST_MAIN(Ut_DuiKeyboardPlugin);

