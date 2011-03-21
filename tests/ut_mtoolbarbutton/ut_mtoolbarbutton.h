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



#ifndef UT_MTOOLBARBUTTON
#define UT_MTOOLBARBUTTON

#include <QtTest/QTest>
#include <QObject>
#include <QSharedPointer>
#include <mtoolbardata.h>
#include <mtoolbarbutton.h>

class MApplication;

class Ut_MToolbarButton : public QObject
{
    Q_OBJECT

private:
    MApplication *app;
    QSharedPointer<MToolbarButton> subject;
    QSharedPointer<MToolbarData> toolbarData;

private slots:
    //! initialize application and class
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void testStylingName_data();
    void testStylingName();
};

#endif
