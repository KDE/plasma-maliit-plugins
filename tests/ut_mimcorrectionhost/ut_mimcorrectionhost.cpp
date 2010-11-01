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



#include "ut_mimcorrectionhost.h"
#include "mimwordtracker.h"
#include "mimwordlist.h"
#include <mplainwindow.h>
#include "utils.h"
#include <QtTest/QTest>
#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QSignalSpy>
#include <MSceneManager>
#include <MSceneWindow>

Q_DECLARE_METATYPE(MImCorrectionHost::CandidateMode)

void Ut_MImCorrectionHost::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_mimcorrectioncandidatewidget",
                                  (char *) "-local-theme" };
    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);

    // MImCorrectionHost uses this internally
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


void Ut_MImCorrectionHost::init()
{
    m_subject = new MImCorrectionHost(parentWindow);

    if (MPlainWindow::instance()->orientationAngle() != M::Angle0) {
        MPlainWindow::instance()->setOrientationAngle(M::Angle0);
        QTest::qWait(1000);
    }
}


void Ut_MImCorrectionHost::cleanup()
{
    delete m_subject;
}

void Ut_MImCorrectionHost::cleanupTestCase()
{
    delete parentWindow;
    delete MPlainWindow::instance();
    delete app;
    app = 0;
}

void Ut_MImCorrectionHost::checkShowWidget()
{
    QStringList candidates;
    candidates << "1" << "2" << "3" << "4" << "5";
    m_subject->setCandidates(candidates);
    // At least, no crash for show and hide
    m_subject->showCorrectionWidget(MImCorrectionHost::WordTrackerMode);
    m_subject->hideCorrectionWidget();
    m_subject->showCorrectionWidget(MImCorrectionHost::WordListMode);
    m_subject->hideCorrectionWidget();
}

void Ut_MImCorrectionHost::checkModes()
{
    QStringList candidates;
    candidates << "1" << "2" << "3" << "4" << "5";
    m_subject->setCandidates(candidates);
    m_subject->showCorrectionWidget(MImCorrectionHost::WordTrackerMode);
    QCOMPARE(m_subject->candidateMode(),  MImCorrectionHost::WordTrackerMode);
    m_subject->showCorrectionWidget(MImCorrectionHost::WordListMode);
    QCOMPARE(m_subject->candidateMode(), MImCorrectionHost::WordListMode);
}

void Ut_MImCorrectionHost::checkCandidatesAndPreedit_data()
{
    QTest::addColumn<QStringList>("candidates");

    QTest::newRow("test1")  << (QStringList() << "1" << "2" << "3" << "4" << "5");
    QTest::newRow("test2")  << (QStringList() << "ab" << "cd" << "ef" << "fg" << "gh");
}

void Ut_MImCorrectionHost::checkCandidatesAndPreedit()
{
    QFETCH(QStringList, candidates);

    m_subject->setCandidates(candidates);
    QCOMPARE(candidates, m_subject->candidates);
}

void Ut_MImCorrectionHost::checkSuggestion_data()
{
    QTest::addColumn<QStringList>("candidates");
    QTest::addColumn<MImCorrectionHost::CandidateMode>("candidateMode");
    QTest::addColumn<QString>("clickedCandidate");

    QTest::newRow("testWordTrackerMode") << (QStringList() << "1" << "2" << "3" << "4" << "5")
                            << MImCorrectionHost::WordTrackerMode
                            << "3";
    QTest::newRow("test2WordListMode") << (QStringList() << "ab" << "cd" << "ef" << "fg" << "gh")
                            << MImCorrectionHost::WordListMode
                            << "gh";
}

void Ut_MImCorrectionHost::checkSuggestion()
{
    QFETCH(QStringList, candidates);
    QFETCH(MImCorrectionHost::CandidateMode, candidateMode);
    QFETCH(QString, clickedCandidate);

    QSignalSpy spy(m_subject, SIGNAL(candidateClicked(const QString &)));

    m_subject->setCandidates(candidates);
    m_subject->showCorrectionWidget(candidateMode);
    // default suggestion is the first one in candidate list which is
    // different with preedit
    QCOMPARE(m_subject->suggestion(), candidates.at(1));

    m_subject->handleCandidateClicked(clickedCandidate);

    // suggestion is the clciked word
    QCOMPARE(m_subject->suggestion(), clickedCandidate);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), clickedCandidate);
}

void Ut_MImCorrectionHost::checkPosition()
{
    QStringList candidates;
    candidates << "fug" << "rug" << "dug" << "tug";
    m_subject->setCandidates(candidates);

    const QSize sceneSize = MPlainWindow::instance()->visibleSceneSize();
    int width = m_subject->wordTracker->idealWidth();

    //set a position to check whether the word tracker is still inside screen.
    QPoint insidePos(sceneSize.width() - width, 100);
    m_subject->setPosition(insidePos);
    QCOMPARE(m_subject->position(), insidePos);

    QPoint outsidepPos(sceneSize.width(), 100);
    m_subject->setPosition(outsidepPos);
    QVERIFY(m_subject->position() != outsidepPos);
    QCOMPARE(m_subject->position(), insidePos);
}

QTEST_APPLESS_MAIN(Ut_MImCorrectionHost);

