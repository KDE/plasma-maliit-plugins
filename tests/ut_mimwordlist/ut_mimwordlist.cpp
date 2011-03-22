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



#include "ut_mimwordlist.h"
#include "mimwordlist.h"
#include "mimwordlistitem.h"
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


void Ut_MImWordList::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_mimcorrectioncandidatewidget",
                                  (char *) "-software" };
    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);
    RegionTracker::createInstance();

    // MImWordList uses this internally
    new MPlainWindow;
    if (MPlainWindow::instance()->orientationAngle() != M::Angle0) {
        MPlainWindow::instance()->setOrientationAngle(M::Angle0);
        QTest::qWait(1000);
    }
}


void Ut_MImWordList::init()
{
    m_subject = new MImWordList;

    if (MPlainWindow::instance()->orientationAngle() != M::Angle0) {
        MPlainWindow::instance()->setOrientationAngle(M::Angle0);
        QTest::qWait(1000);
    }
}


void Ut_MImWordList::cleanup()
{
    delete m_subject;
}

void Ut_MImWordList::cleanupTestCase()
{
    delete MPlainWindow::instance();
    RegionTracker::destroyInstance();
    delete app;
    app = 0;
}

void Ut_MImWordList::testCandidates_data()
{
    QTest::addColumn<QStringList>("candidates");

    QTest::newRow("test1")  << (QStringList() << "1" << "2" << "3" << "4" << "5");
    QTest::newRow("test2")  << (QStringList() << "abc" << "def" << "ghi" << "jfk" << "lmn");
}

void Ut_MImWordList::testCandidates()
{
    QFETCH(QStringList, candidates);
    m_subject->setCandidates(candidates, false);
    QCOMPARE(m_subject->candidates(), candidates);
}

void Ut_MImWordList::testSelect()
{
    QStringList candidates = (QStringList() << "abc" << "def" << "ghi" << "jfk" << "lmn");
    m_subject->setCandidates(candidates, false);
    m_subject->appear();
    QTest::qWait(600);
    QVERIFY(m_subject->isVisible());
    if (m_subject->sceneWindowState() == MSceneWindow::Appearing) {
        QSKIP("word list is during appearing animation", SkipSingle);
    }
    QSignalSpy clickSpy(m_subject, SIGNAL(candidateClicked(const QString &)));

    m_subject->candidateItems[2]->click();

    QCOMPARE(clickSpy.count(), 1);
    QCOMPARE(clickSpy.first().first().toString(), QString("ghi"));
    m_subject->disappear();
}

QTEST_APPLESS_MAIN(Ut_MImWordList);

