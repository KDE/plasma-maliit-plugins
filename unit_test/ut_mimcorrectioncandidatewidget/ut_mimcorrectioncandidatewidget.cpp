/* * This file is part of dui-keyboard *
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



#include "ut_duiimcorrectioncandidatewidget.h"
#include "duiplainwindow.h"
#include <DuiTheme>
#include <DuiList>
#include <QtTest/QTest>
#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QStringListModel>
#include <QSignalSpy>
#include <QGraphicsSceneMouseEvent>
#include <DuiSceneManager>

void Ut_DuiImCorrectionCandidateWidget::initTestCase()
{
    static int dummyArgc = 1;
    static char *dummyArgv[1] = { (char *) "./ut_duiimcorrectioncandidatewidget" };
    // Avoid waiting if im server is not responding
    DuiApplication::setLoadDuiInputContext(false);
    app = new DuiApplication(dummyArgc, dummyArgv);

    DuiTheme::instance()->loadCSS("/usr/share/dui/virtual-keyboard/css/864x480.css");

    // DuiImCorrectionCandidateWidget uses this internally
    new DuiPlainWindow;
    if (DuiPlainWindow::instance()->orientationAngle() != Dui::Angle0) {
        DuiPlainWindow::instance()->setOrientationAngle(Dui::Angle0);
        QTest::qWait(1000);
    }

    // initialize testCandidateWidgetSize
    m_subject = new DuiImCorrectionCandidateWidget();
    QStringList candidates;
    candidates << "1" << "2" << "3";
    m_subject->setCandidates(candidates);
    testCandidateWidgetSize = m_subject->candidatesWidget->preferredSize().toSize();
    testCandidateWidgetSize.setWidth(m_subject->candidateWidth);
    delete m_subject;
}


void Ut_DuiImCorrectionCandidateWidget::init()
{
    m_subject = new DuiImCorrectionCandidateWidget();

    if (DuiPlainWindow::instance()->orientationAngle() != Dui::Angle0) {
        DuiPlainWindow::instance()->setOrientationAngle(Dui::Angle0);
        QTest::qWait(1000);
    }
}


void Ut_DuiImCorrectionCandidateWidget::cleanup()
{
    delete m_subject;
}

void Ut_DuiImCorrectionCandidateWidget::cleanupTestCase()
{
    delete DuiPlainWindow::instance();
    delete app;
    app = 0;
}

void Ut_DuiImCorrectionCandidateWidget::checkPositionByPoint_data()
{
    QTest::addColumn<QPoint>("pos");
    QTest::addColumn<int>("bottomLimit");
    QTest::addColumn<QPoint>("expected");

    const int sceneHeight = DuiPlainWindow::instance()->visibleSceneSize().height();

    QTest::newRow("null")      << QPoint() << -1 << QPoint();
    QTest::newRow("origo")     << QPoint(0, 0) << -1 << QPoint(0, 0);
    QTest::newRow("negative")  << QPoint(-10, -10) << -1 << QPoint(0, 0);
    QTest::newRow("positive1") << QPoint(10, 10) << -1 << QPoint(10, 10);
    QTest::newRow("positive2") << QPoint(80, 80) << -1 << QPoint(80, 80);

    // Goes over sceneHeight limit
    QTest::newRow("positive3") <<QPoint(700, sceneHeight - 50) << -1
        << QPoint(700, (sceneHeight - testCandidateWidgetSize.height()));

    // No room above so aligns to y=0.
    QTest::newRow("bottom limit 1") << QPoint(10, 10) << 4 << QPoint(10, 0);

    // There is room above so bottom limit holds (y+height=bottomlimit).
    QTest::newRow("bottom limit 2") << QPoint(10, 250) << 220 << QPoint(10, 220 - testCandidateWidgetSize.height());
}

void Ut_DuiImCorrectionCandidateWidget::checkPositionByPoint()
{
    QStringList candidates;
    candidates << "1" << "2" << "3";
    m_subject->setCandidates(candidates);
    QFETCH(QPoint, pos);
    QFETCH(int, bottomLimit);
    QFETCH(QPoint, expected);

    m_subject->setPosition(pos, bottomLimit);
    QCOMPARE(m_subject->position(), expected);
}


void Ut_DuiImCorrectionCandidateWidget::checkPositionByPreeditRect()
{
    const int CandidatesPreeditMargin = 10;
    const QSize sceneSize = DuiPlainWindow::instance()->visibleSceneSize();

    QList<QRect> rects;
    QList<QPoint> positionsCheck;
    QPoint pos;
    QSize size;

    QStringList candidates;
    candidates << "1" << "2" << "3";
    m_subject->setCandidates(candidates);

    // check with null rectangle
    rects.append(QRect());
    positionsCheck.append(QPoint());
    rects.append(QRect(0, 0, 0, 0));
    positionsCheck.append(QPoint(0, 0));
    rects.append(QRect(QPoint(10, 10), QSize(0, 0)));
    positionsCheck.append(QPoint(0, 0));

    // invalid rectangle
    rects.append(QRect(QPoint(10, 10), QPoint(5, 5)));
    positionsCheck.append(QPoint());
    rects.append(QRect(QPoint(), QPoint(-100, -100)));
    positionsCheck.append(QPoint());
    rects.append(QRect(QPoint(0, 0), QPoint(5, -5)));
    positionsCheck.append(QPoint());

    // valid rectangle inside area
    pos = QPoint(10, 10 + testCandidateWidgetSize.height());
    size = QSize(1, 1);
    rects.append(QRect(pos, size));
    positionsCheck.append(QPoint(pos.x() + size.width() + CandidatesPreeditMargin,
                                 pos.y() + size.height() / 2 - testCandidateWidgetSize.height() / 2));

    pos = QPoint(4, 5 + testCandidateWidgetSize.height());
    size = QSize(100, 5);
    rects.append(QRect(pos, size));
    positionsCheck.append(QPoint(pos.x() + size.width() + CandidatesPreeditMargin,
                                 pos.y() + size.height() / 2 - testCandidateWidgetSize.height() / 2));

    // valid rectangle outside area
    size = QSize(100, 5);
    pos = QPoint(0, testCandidateWidgetSize.height() / 2 - size.height() / 2 - 1);
    rects.append(QRect(pos, size));
    positionsCheck.append(QPoint(pos.x() + size.width() + CandidatesPreeditMargin, 0));

    size = QSize(10, 50);
    pos = QPoint(0, testCandidateWidgetSize.height() / 2 - size.height() / 2 - 1);
    rects.append(QRect(pos, size));
    positionsCheck.append(QPoint(pos.x() + size.width() + CandidatesPreeditMargin, 0));

    // switch to left side, vertical overlap
    size = QSize(10, 10);
    pos = QPoint(sceneSize.width() - testCandidateWidgetSize.width() + 1,
                 sceneSize.height() - testCandidateWidgetSize.height() / 2);
    rects.append(QRect(pos, size));
    positionsCheck.append(QPoint(pos.x() - CandidatesPreeditMargin - testCandidateWidgetSize.width(),
                                 sceneSize.height() - testCandidateWidgetSize.height()));

    for (int i = 0; i < rects.size(); ++i) {
        m_subject->setPosition(rects.at(i));
        QCOMPARE(m_subject->position(), positionsCheck.at(i));
    }
}

void Ut_DuiImCorrectionCandidateWidget::checkActiveIndex()
{
    QStringList candidates;
    candidates << "1" << "2" << "3" << "4" << "5"
        << "ab" << "cd" << "ef" << "gf";
    m_subject->setPreeditString(candidates.last());
    m_subject->setCandidates(candidates);
    //default preedit is not in the list
    QCOMPARE(m_subject->candidatesModel->rowCount(), (candidates.count() - 1));
    QCOMPARE(m_subject->activeIndex(), -1);

    QList<int> index;
    index << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7;

    for (int i = 0; i < index.size(); ++i) {
        m_subject->setPreeditString(candidates.at(i));
        QCOMPARE(m_subject->activeIndex(), index.at(i));
    }
}


void Ut_DuiImCorrectionCandidateWidget::checkPreeditString()
{
    QStringList str;
    str << "autobahnraststaettenbetreiber" << "Nobody_is_there" <<
        "99ab88bc!§%/()=?" << "lalallala";

    foreach(const QString & tmp, str) {
        m_subject->setPreeditString(tmp);
        QVERIFY(m_subject->preeditString() == tmp);
    }
}

void Ut_DuiImCorrectionCandidateWidget::setCandidatesAndSelect()
{
    QStringList candidates;
    QStringList expectedWord;
    QList<QPoint> positions;
    QList<int> expectedSignal;
    QGraphicsSceneMouseEvent *press = 0;
    QGraphicsSceneMouseEvent *release = 0;
    int rowHeight = testCandidateWidgetSize.height() / 3;

    candidates << "1" << "2" << "3";
    expectedWord << "" << "1" << "2" << "3" << "";
    positions << QPoint(2, -50)
              << QPoint(2, rowHeight / 2)
              << QPoint(2, rowHeight * 3 / 2)
              << QPoint(2, rowHeight * 5 / 2)
              << QPoint(800, rowHeight / 2)
              << QPoint(-20, rowHeight / 2);
    expectedSignal << 0 << 1 << 1 << 0 << 0 << 0;

    for (int n = 0; n < positions.count(); ++n) {
        qDebug() << "Test position" << positions.at(n);
        press = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMousePress);
        release = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMouseRelease);
        press->setPos(positions.at(n));
        release->setPos(positions.at(n));

        delete m_subject;
        m_subject = new DuiImCorrectionCandidateWidget();
        QSignalSpy spyRegion(m_subject, SIGNAL(regionUpdated(const QRegion &)));
        QSignalSpy spyCandidate(m_subject, SIGNAL(candidateClicked(const QString &)));
        QSignalSpy spyHidden(m_subject, SIGNAL(hidden()));

        //initialization
        m_subject->setPreeditString(candidates.last());
        m_subject->setCandidates(candidates);
        m_subject->setPosition(QPoint(0, 0));
        m_subject->showWidget();
        QStringList filteredCandidates = candidates;
        filteredCandidates.removeOne(candidates.last());
        QVERIFY(m_subject->candidates() == filteredCandidates);
        //actual testing
        QCOMPARE(spyRegion.count(), 1);

        QRectF candidatesWidgetRect(m_subject->position(), m_subject->candidatesWidget->preferredSize());
        if (candidatesWidgetRect.contains(positions.at(n))) {
            int index = positions.at(n).y() / rowHeight;
            if (index >= 0 && index < m_subject->candidatesModel->rowCount()) {
                m_subject->candidatesWidget->selectItem(m_subject->candidatesWidget->itemModel()->index(index, 0));
            }
        } else {
            m_subject->mousePressEvent(press);
            m_subject->mouseMoveEvent(press);
        }
        //if we are not crashed then it is ok

        m_subject->mouseReleaseEvent(release);
        QCOMPARE(spyCandidate.count(), expectedSignal.at(n));
        if (spyCandidate.count() > 0) {
            QCOMPARE(spyCandidate.first().count(), 1);
            QCOMPARE(spyCandidate.first().first().toString(), expectedWord.at(n));
        }
        QCOMPARE(spyHidden.count(), 1);

        delete press;
        delete release;
    }
}

void Ut_DuiImCorrectionCandidateWidget::checkShowWidget()
{
    QStringList candidates;
    candidates << "1" << "2" << "3" << "4" << "5";
    m_subject->setCandidates(candidates);
    // At least, no crash for show and hide
    m_subject->showWidget();
    m_subject->hide();
}

QTEST_APPLESS_MAIN(Ut_DuiImCorrectionCandidateWidget);

