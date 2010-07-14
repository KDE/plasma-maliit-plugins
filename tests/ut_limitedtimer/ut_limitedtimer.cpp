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



#include "ut_limitedtimer.h"
#include <limitedtimer.h>

#include <MApplication>
#include <MTheme>
#include <QSignalSpy>

void Ut_LimitedTimer::initTestCase()
{
    static int argc = 2;
    static char *app_name[] = { (char *)"ut_limitedtimer",
                                (char *) "-local-theme" };

    // Avoid waiting if im server is not responding
    MApplication::setLoadMInputContext(false);
    app = new MApplication(argc, app_name);
}

void Ut_LimitedTimer::cleanupTestCase()
{
    delete app;
    app = 0;
}

void Ut_LimitedTimer::init()
{
    subject = new LimitedTimer;
}

void Ut_LimitedTimer::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_LimitedTimer::testSingleShot()
{
    QSignalSpy spy(subject, SIGNAL(timeout()));

    subject->setSingleShot(true);
    subject->setInterval(200);

    QVERIFY(subject->isActive() == false);
    subject->start();
    QVERIFY(subject->isActive() == true);
    QTest::qWait(100);
    QVERIFY(spy.count() == 0);
    QTest::qWait(150);
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.at(0).count() == 0);

    QVERIFY(subject->isActive() == false);
    subject->start(-20);
    QVERIFY(subject->isActive() == false);

    spy.clear();
    subject->start(200);
    QVERIFY(subject->isActive() == true);
    QTest::qWait(100);
    QVERIFY(spy.count() == 0);
    QTest::qWait(150);
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.at(0).count() == 0);
}

void Ut_LimitedTimer::testCallLimit()
{
    QSignalSpy spy(subject, SIGNAL(timeout()));

    subject->setSingleShot(true);
    subject->setInterval(200);

    subject->start();
    QVERIFY(subject->isActive() == true);
    subject->stop();
    QVERIFY(subject->isActive() == false);
    subject->start();
    QVERIFY(subject->isActive() == false);
    QTest::qWait(50); //this value depends on internal limit in LimitedTimer
    subject->start();
    QVERIFY(subject->isActive() == false);
    QTest::qWait(200);
    QVERIFY(spy.count() == 0);

    spy.clear();
    subject->start(200);
    QVERIFY(subject->isActive() == true);
    subject->stop();
    QVERIFY(subject->isActive() == false);
    subject->start(200);
    QVERIFY(subject->isActive() == false);
    QTest::qWait(50); //this value depends on internal limit in LimitedTimer
    subject->start(200);
    QVERIFY(subject->isActive() == false);
    QTest::qWait(200);
    QVERIFY(spy.count() == 0);
}

QTEST_APPLESS_MAIN(Ut_LimitedTimer);
