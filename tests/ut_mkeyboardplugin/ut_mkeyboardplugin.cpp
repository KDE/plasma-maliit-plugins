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



#include "ut_mkeyboardplugin.h"
#include "utils.h"

#include <QtTest/QTest>
#include <QObject>
#include <QDebug>

void Ut_MKeyboardPlugin::initTestCase()
{
    disableQtPlugins();
    // check that Plugin can be accessed
    m_subject = new MKeyboardPlugin;
}


void Ut_MKeyboardPlugin::cleanupTestCase()
{
    delete m_subject;
    m_subject = 0;
}


void Ut_MKeyboardPlugin::init()
{
}


void Ut_MKeyboardPlugin::cleanup()
{
}


void Ut_MKeyboardPlugin::testMethods()
{
    // some simple checks that the functions seems to work
    // check name of plugin
    QVERIFY(m_subject->name() == "MeegoKeyboard");
}


QTEST_MAIN(Ut_MKeyboardPlugin);

