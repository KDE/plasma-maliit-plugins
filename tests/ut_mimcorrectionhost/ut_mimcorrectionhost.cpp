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
#include "abstractenginewidgethost.h"
#include "mkeyboardhost.h"
#include "enginemanager.h"
#include "mimwordtracker.h"
#include "mimwordlist.h"
#include "reactionmappainter.h"
#include "utils.h"
#include "minputmethodhoststub.h"

#include <mplainwindow.h>
#include <QtTest/QTest>
#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QSignalSpy>
#include <MSceneManager>
#include <MSceneWindow>

Q_DECLARE_METATYPE(AbstractEngineWidgetHost::DisplayMode)

void Ut_MImCorrectionHost::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_mimcorrectioncandidatewidget",
                                  (char *) "-software" };
    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);

    inputMethodHost = new MInputMethodHostStub;
    mainWindow = new QWidget;
    keyboardHost = new MKeyboardHost(inputMethodHost, mainWindow);

    // MImCorrectionHost uses this internally
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
    delete keyboardHost;
    delete mainWindow;
    delete inputMethodHost;
    delete app;
    app = 0;
}

void Ut_MImCorrectionHost::checkShowWidget()
{
    QStringList candidates;
    candidates << "1" << "2" << "3" << "4" << "5";
    m_subject->setCandidates(candidates);
    // At least, no crash for show and hide
    m_subject->showEngineWidget(AbstractEngineWidgetHost::FloatingMode);
    m_subject->hideEngineWidget();
    m_subject->showEngineWidget(AbstractEngineWidgetHost::DialogMode);
    m_subject->hideEngineWidget();
}

void Ut_MImCorrectionHost::checkModes()
{
    QStringList candidates;
    candidates << "1" << "2" << "3" << "4" << "5";
    m_subject->setCandidates(candidates);
    m_subject->showEngineWidget(AbstractEngineWidgetHost::FloatingMode);
    QCOMPARE(m_subject->displayMode(),  AbstractEngineWidgetHost::FloatingMode);
    m_subject->showEngineWidget(AbstractEngineWidgetHost::DialogMode);
    QCOMPARE(m_subject->displayMode(), AbstractEngineWidgetHost::DialogMode);
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
    QCOMPARE(candidates, m_subject->candidates());
}

void Ut_MImCorrectionHost::checkSuggestion_data()
{
    QTest::addColumn<QStringList>("candidates");
    QTest::addColumn<AbstractEngineWidgetHost::DisplayMode>("displayMode");
    QTest::addColumn<QString>("clickedCandidate");

    QTest::newRow("testWordTrackerMode") << (QStringList() << "1" << "2" << "3" << "4" << "5")
                            << AbstractEngineWidgetHost::FloatingMode
                            << "3";
    QTest::newRow("testWordListMode") << (QStringList() << "ab" << "cd" << "ef" << "fg" << "gh")
                            << AbstractEngineWidgetHost::DialogMode
                            << "gh";
}

void Ut_MImCorrectionHost::checkSuggestion()
{
    QFETCH(QStringList, candidates);
    QFETCH(AbstractEngineWidgetHost::DisplayMode, displayMode);
    QFETCH(QString, clickedCandidate);

    QSignalSpy spy(m_subject, SIGNAL(candidateClicked(const QString &, int)));

    m_subject->setCandidates(candidates);
    m_subject->showEngineWidget(displayMode);
    if (displayMode == AbstractEngineWidgetHost::DialogMode) {
        QTest::qWait(600);
    }
    QCOMPARE(m_subject->candidates(), candidates);
    // default suggestion is the first one in candidate list which is
    // different with preedit
    QCOMPARE(m_subject->suggestedWordIndex(), 1);

    m_subject->handleCandidateClicked(clickedCandidate);

    // suggestion is the clciked word
    QCOMPARE(m_subject->suggestedWordIndex(), candidates.indexOf(clickedCandidate));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), clickedCandidate);
}

void Ut_MImCorrectionHost::checkpendingCandidatesUpdate_data()
{
    QTest::addColumn<QStringList>("candidates");
    QTest::addColumn<AbstractEngineWidgetHost::DisplayMode>("displayMode");
    QTest::addColumn<QStringList>("updatedCandidates");

    QTest::newRow("testWordTrackerMode") << (QStringList() << "1" << "2" << "3" << "4" << "5")
                            << AbstractEngineWidgetHost::FloatingMode
                            << (QStringList() << "ab" << "cd" << "ef" << "fg" << "gh");
    QTest::newRow("testWordListMode") << (QStringList() << "ab" << "cd" << "ef" << "fg" << "gh")
                            << AbstractEngineWidgetHost::DialogMode
                            << (QStringList() << "1" << "2" << "3" << "4" << "5");
}

void Ut_MImCorrectionHost::checkpendingCandidatesUpdate()
{
    QFETCH(QStringList, candidates);
    QFETCH(AbstractEngineWidgetHost::DisplayMode, displayMode);
    QFETCH(QStringList, updatedCandidates);

    QCOMPARE(m_subject->isActive(), false);
    QCOMPARE(m_subject->pendingCandidatesUpdate, false);
    m_subject->setCandidates(candidates);
    QCOMPARE(m_subject->pendingCandidatesUpdate, true);

    QVERIFY(m_subject->candidates() != m_subject->wordList->candidates());

    m_subject->showEngineWidget(displayMode);
    QTest::qWait(600);

    QCOMPARE(m_subject->pendingCandidatesUpdate, false);
    // only update the wordtracker or wordlist according current mode
    if (displayMode == AbstractEngineWidgetHost::FloatingMode) {
        QVERIFY(m_subject->candidates() != m_subject->wordList->candidates());
    } else {
        QVERIFY(m_subject->candidates() == m_subject->wordList->candidates());
    }

    m_subject->setCandidates(updatedCandidates);
    QCOMPARE(m_subject->pendingCandidatesUpdate, false);

    if (displayMode == AbstractEngineWidgetHost::FloatingMode) {
        QVERIFY(m_subject->candidates() != m_subject->wordList->candidates());
    } else {
        QVERIFY(m_subject->candidates() == m_subject->wordList->candidates());
    }
}

QTEST_APPLESS_MAIN(Ut_MImCorrectionHost);

