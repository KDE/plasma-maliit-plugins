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



#ifndef UT_DUIVIRTUALKEYBOARDPLUGIN_H
#define UT_DUIVIRTUALKEYBOARDPLUGIN_H


#include "duikeyboardplugin.h"
#include "duiapplication.h"

#include <QtTest/QTest>
#include <QObject>

class Ut_DuiKeyboardPlugin : public QObject
{
    Q_OBJECT

private:
    DuiApplication *app;
    DuiKeyboardPlugin *m_subject;

private slots:
    //! initialize application and class
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void testMethods();
};

#endif
