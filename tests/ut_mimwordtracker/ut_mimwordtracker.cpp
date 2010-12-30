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



#include "ut_mimwordtracker.h"
#include "mimwordtracker.h"
#include "mplainwindow.h"
#include "utils.h"
#include <QtTest/QTest>
#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QSignalSpy>
#include <QGraphicsSceneMouseEvent>
#include <MSceneManager>
#include <regiontracker.h>

void Ut_MImWordTracker::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_mimcorrectioncandidatewidget",
                                  (char *) "-software" };
    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);
    RegionTracker::createInstance();

    // MImWordTracker uses this internally
    new MPlainWindow;
    if (MPlainWindow::instance()->orientationAngle() != M::Angle0) {
        MPlainWindow::instance()->setOrientationAngle(M::Angle0);
        QTest::qWait(1000);
    }
    parentWindow = new MSceneWindow;
    parentWindow->setManagedManually(true); // we want the scene window to remain in origin
    // Adds scene window to scene.
    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(parentWindow);
}


void Ut_MImWordTracker::init()
{
    m_subject = new MImWordTracker(parentWindow);

    if (MPlainWindow::instance()->orientationAngle() != M::Angle0) {
        MPlainWindow::instance()->setOrientationAngle(M::Angle0);
        QTest::qWait(1000);
    }
}


void Ut_MImWordTracker::cleanup()
{
    delete m_subject;
}

void Ut_MImWordTracker::cleanupTestCase()
{
    delete parentWindow;
    delete MPlainWindow::instance();
    RegionTracker::destroyInstance();
    delete app;
    app = 0;
}

void Ut_MImWordTracker::testCandidate_data()
{
    QTest::addColumn<QString>("candidate");

    QTest::newRow("test1")  << "1";
    QTest::newRow("test2")  << "abc";
}

void Ut_MImWordTracker::testCandidate()
{
    QFETCH(QString, candidate);
    m_subject->setCandidate(candidate);
    QCOMPARE(m_subject->candidate(), candidate);
}

void Ut_MImWordTracker::testAppearAndDisappear()
{
    m_subject->appear(false);
    QVERIFY(m_subject->isVisible());
    m_subject->disappear(false);
    QVERIFY(!m_subject->isVisible());
}

void Ut_MImWordTracker::testSelect()
{
    QString candidate("test");
    m_subject->appear(false);
    QVERIFY(m_subject->isVisible());
    m_subject->setCandidate(candidate);

    QSignalSpy clickSpy(m_subject, SIGNAL(candidateClicked(const QString &)));
    m_subject->select();
    QCOMPARE(clickSpy.count(), 1);
    QCOMPARE(clickSpy.first().first().toString(), candidate);
}

void Ut_MImWordTracker::testLongTap()
{
    QString candidate("test");
    m_subject->appear(false);
    QVERIFY(m_subject->isVisible());
    m_subject->setCandidate(candidate);

    QSignalSpy clickSpy(m_subject, SIGNAL(longTapped()));
    m_subject->longTap();
    QCOMPARE(clickSpy.count(), 1);
}


QTEST_APPLESS_MAIN(Ut_MImWordTracker);

