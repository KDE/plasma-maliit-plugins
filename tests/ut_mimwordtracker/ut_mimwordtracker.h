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



#ifndef UT_MIMWORDTRACKER_H
#define UT_MIMWORDTRACKER_H

#include "mimwordtracker.h"
#include "mapplication.h"
#include <QtTest/QTest>
#include <QObject>

class MSceneWindow;

class Ut_MImWordTracker : public QObject
{
    Q_OBJECT

private:
    MApplication *app;
    MImWordTracker *m_subject;
    MSceneWindow *parentWindow;

private slots:
    //! initialize application and class
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();
    void testCandidate_data();
    void testCandidate();
    void testAppearAndDisappear();
    void testSelect();
    void testLongTap();
};

#endif
