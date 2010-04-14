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



#ifndef UT_TOOLBARMANAGER_H
#define UT_TOOLBARMANAGER_H

#include <QObject>
#include <QtTest/QTest>

class MApplication;
class ToolbarManager;
class MImToolbar;
class MVirtualKeyboardStyleContainer;

class Ut_ToolbarManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void testLoadToolbar();

private:
    MApplication *app;
    ToolbarManager *subject;
    MImToolbar *m_parent;
    MVirtualKeyboardStyleContainer *style;
};

#endif

