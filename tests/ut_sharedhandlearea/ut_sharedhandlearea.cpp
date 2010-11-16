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

#include "ut_sharedhandlearea.h"
#include <utils.h>
#include <sharedhandlearea.h>
#include <mgconfitem_stub.h>
#include <mplainwindow.h>
#include <MSceneWindow>

#include <MApplication>
#include <MScene>

namespace
{
    const QString TargetSettingsName("/meegotouch/target/name");
    const QString DefaultTargetName("Default");
}

void Ut_SharedHandleArea::initTestCase()
{
    static int dummyArgc = 3;
    static char *dummyArgv[3] = { (char *) "./ut_sharedhandlearea",
        (char *) "-local-theme",
        (char *) "-software"  };
    // this value is required by the theme daemon
    MGConfItem(TargetSettingsName).set(DefaultTargetName);

    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);

    sceneWindow = createMSceneWindow(new MPlainWindow); // also create singleton

    parent = new QGraphicsWidget;
    MPlainWindow::instance()->scene()->addItem(parent);
}

void Ut_SharedHandleArea::cleanupTestCase()
{
    delete parent;
    delete sceneWindow;
    delete MPlainWindow::instance();
    delete app;
}

void Ut_SharedHandleArea::init()
{
    imToolbar = new MImToolbar;
    subject = new SharedHandleArea(*imToolbar, parent);
}

void Ut_SharedHandleArea::cleanup()
{
    delete subject;
    delete imToolbar;
}

void Ut_SharedHandleArea::testWatching()
{
    QGraphicsWidget *widget1 = new QGraphicsWidget(parent);
    QGraphicsWidget *widget2 = new QGraphicsWidget(parent);
    QRectF pos1(10, 20, 30, 40);
    QRectF pos2(10, 200, 30, 40);

    widget1->setGeometry(pos1);
    widget2->setGeometry(pos2);

    subject->watchOnWidget(widget1);
    subject->watchOnWidget(widget2);

    QCOMPARE(subject->geometry().bottom(), widget1->geometry().top());

    pos1.setTop(30);
    widget1->setGeometry(pos1);
    QCOMPARE(subject->geometry().bottom(), widget1->geometry().top());

    pos2.setTop(100);
    widget2->setGeometry(pos2);
    QCOMPARE(subject->geometry().bottom(), widget1->geometry().top());

    widget1->hide();
    QCOMPARE(subject->geometry().bottom(), widget2->geometry().top());

    widget1->show();
    QCOMPARE(subject->geometry().bottom(), widget1->geometry().top());

    delete widget1;
    delete widget2;
}

QTEST_APPLESS_MAIN(Ut_SharedHandleArea)

