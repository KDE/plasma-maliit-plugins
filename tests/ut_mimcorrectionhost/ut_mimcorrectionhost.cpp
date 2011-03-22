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
#include "reactionmappainter.h"
#include "utils.h"

#include <mplainwindow.h>
#include <QtTest/QTest>
#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QSignalSpy>
#include <MSceneManager>
#include <MSceneWindow>
#include <regiontracker.h>

Q_DECLARE_METATYPE(MImCorrectionHost::CandidateMode)

void Ut_MImCorrectionHost::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_mimcorrectioncandidatewidget",
                                  (char *) "-software" };
    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);
    RegionTracker::createInstance();
    ReactionMapPainter::createInstance();

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
    m_subject = new MImCorrectionHost(0, parentWindow);

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
    RegionTracker::destroyInstance();
    ReactionMapPainter::destroyInstance();
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
    QTest::newRow("testWordListMode") << (QStringList() << "ab" << "cd" << "ef" << "fg" << "gh")
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
    if (candidateMode == MImCorrectionHost::WordListMode) {
        QTest::qWait(600);
    }
    // default suggestion is the first one in candidate list which is
    // different with preedit
    QCOMPARE(m_subject->suggestion(), candidates.at(1));

    m_subject->handleCandidateClicked(clickedCandidate);

    // suggestion is the clciked word
    QCOMPARE(m_subject->suggestion(), clickedCandidate);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), clickedCandidate);
}

void Ut_MImCorrectionHost::checkpendingCandidatesUpdate_data()
{
    QTest::addColumn<QStringList>("candidates");
    QTest::addColumn<MImCorrectionHost::CandidateMode>("candidateMode");
    QTest::addColumn<QStringList>("updatedCandidates");

    QTest::newRow("testWordTrackerMode") << (QStringList() << "1" << "2" << "3" << "4" << "5")
                            << MImCorrectionHost::WordTrackerMode
                            << (QStringList() << "ab" << "cd" << "ef" << "fg" << "gh");
    QTest::newRow("testWordListMode") << (QStringList() << "ab" << "cd" << "ef" << "fg" << "gh")
                            << MImCorrectionHost::WordListMode
                            << (QStringList() << "1" << "2" << "3" << "4" << "5");
}

void Ut_MImCorrectionHost::checkpendingCandidatesUpdate()
{
    QFETCH(QStringList, candidates);
    QFETCH(MImCorrectionHost::CandidateMode, candidateMode);
    QFETCH(QStringList, updatedCandidates);

    QCOMPARE(m_subject->isActive(), false);
    QCOMPARE(m_subject->pendingCandidatesUpdate, false);
    m_subject->setCandidates(candidates);
    QCOMPARE(m_subject->pendingCandidatesUpdate, true);

    QVERIFY(m_subject->suggestion() != m_subject->wordTracker->candidate());
    QVERIFY(m_subject->candidates != m_subject->wordList->candidates());

    m_subject->showCorrectionWidget(candidateMode);
    QTest::qWait(600);

    QCOMPARE(m_subject->pendingCandidatesUpdate, false);
    // only update the wordtracker or wordlist according current mode
    if (candidateMode == MImCorrectionHost::WordTrackerMode) {
        QVERIFY(m_subject->suggestion() == m_subject->wordTracker->candidate());
        QVERIFY(m_subject->candidates != m_subject->wordList->candidates());
    } else {
        QVERIFY(m_subject->suggestion() != m_subject->wordTracker->candidate());
        QVERIFY(m_subject->candidates == m_subject->wordList->candidates());
    }

    m_subject->setCandidates(updatedCandidates);
    QCOMPARE(m_subject->pendingCandidatesUpdate, false);

    if (candidateMode == MImCorrectionHost::WordTrackerMode) {
        QVERIFY(m_subject->suggestion() == m_subject->wordTracker->candidate());
        QVERIFY(m_subject->candidates != m_subject->wordList->candidates());
    } else {
        QVERIFY(m_subject->suggestion() != m_subject->wordTracker->candidate());
        QVERIFY(m_subject->candidates == m_subject->wordList->candidates());
    }
}

QTEST_APPLESS_MAIN(Ut_MImCorrectionHost);

