/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list 
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list 
 * of conditions and the following disclaimer in the documentation and/or other materials 
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be 
 * used to endorse or promote products derived from this software without specific 
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

