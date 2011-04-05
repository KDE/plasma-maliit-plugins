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



#include "ut_wordribbonitem.h"
#include "wordribbonitem.h"
#include "wordribbonitemstyle.h"
#include "mwidget.h"
#include "mplainwindow.h"
#include "utils.h"
#include <MTheme>
#include <QtTest/QTest>
#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QSignalSpy>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsWidget>
#include <MSceneManager>

namespace {
    const int SceneRotationTime = 1400; // in ms
}

void Ut_WordRibbonItem::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_mimcorrectioncandidatewidget",
                                  (char *) "-local-theme" };
    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);
    view = new MPlainWindow;
    candidateItemStyleContainer = new WordRibbonItemStyleContainer;
//    candidateItemStyleContainer->initialize("WordRibbonItem", "WordRibbonItemView", 0);
}


void Ut_WordRibbonItem::init()
{
    subject = new WordRibbonItem(WordRibbon::RibbonStyleMode, 0);

//    if (MPlainWindow::instance()->orientationAngle() != M::Angle0)
//        rotateToAngle(M::Angle0);
}


void Ut_WordRibbonItem::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_WordRibbonItem::cleanupTestCase()
{
    delete view;
    view = 0;
    delete app;
    app = 0;
}

void Ut_WordRibbonItem::checkSetText_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("expected");

    QTest::newRow("text1") << QString("") << QString("");
    QTest::newRow("text2") << QString("a") << QString("a");
    QTest::newRow("text3") << QString("ab") << QString("ab");
    QTest::newRow("text4") << QString("abc") << QString("abc");

}

void Ut_WordRibbonItem::checkSetText()
{
    QFETCH(QString, text);
    QFETCH(QString, expected);

    subject->setText(text);
    QCOMPARE(subject->text(), expected);

    subject->clearText();
    QCOMPARE(subject->text(), QString(""));
}
void Ut_WordRibbonItem::checkSetHighlighted()
{
    subject->enableHighlight();
    subject->highlight();
    QCOMPARE(subject->highlighted(), true);

    subject->disableHighlight();
    QCOMPARE(subject->highlighted(), false);
}

void Ut_WordRibbonItem::checkSetPositionIndex_data()
{
    QTest::addColumn<int>("positionIndex");
    QTest::addColumn<int>("expected");

    QTest::newRow("text1") << -10 << -10;
    QTest::newRow("text2") << 2 << 2;
    QTest::newRow("text3") << 2999 << 2999;
    QTest::newRow("text4") << 10000 << 10000;
}

void Ut_WordRibbonItem::checkSetPositionIndex()
{
    QFETCH(int, positionIndex);
    QFETCH(int, expected);

    subject->setPositionIndex(positionIndex);
    QCOMPARE(subject->positionIndex(), expected);
}

void Ut_WordRibbonItem::checkSignalEmission_data()
{
    QTest::addColumn<QPointF>("mousePress");
    QTest::addColumn<QPointF>("mouseRelease");
    QTest::addColumn<bool>("expectedPressSignalEmission");
    QTest::addColumn<bool>("expectedReleaseSignalEmission");

    QTest::newRow("pressInside&&releaseInsideCancelRect")
            << QPointF(10, 32) <<QPointF(10, 10)
            <<true <<true;

    QTest::newRow("pressInside&&releaseOutsideCancelRect")
            << QPointF(10, 32) <<QPointF(-50, -50)
            <<true <<false;
}

void Ut_WordRibbonItem::checkSignalEmission()
{
    qDebug() <<QTest::currentDataTag();
    QFETCH(QPointF, mousePress);
    QFETCH(QPointF, mouseRelease);
    QFETCH(bool, expectedPressSignalEmission);
    QFETCH(bool, expectedReleaseSignalEmission);

    QSignalSpy spyPress(subject, SIGNAL(mousePressed()));
    QSignalSpy spyRelease(subject, SIGNAL(mouseReleased()));
    QCOMPARE(spyPress.count(), 0);
    QCOMPARE(spyRelease.count(), 0);

    subject->setText("a");
    subject->show();
    qDebug() <<"subject bounding rect = " <<subject->boundingRect();

    QGraphicsSceneMouseEvent *press = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMousePress);
    QGraphicsSceneMouseEvent *move = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMouseMove);
    QGraphicsSceneMouseEvent *release = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMouseRelease);
    press->setPos(mousePress);
    move->setPos(mouseRelease);
    release->setPos(mouseRelease);

    subject->mousePressEvent(press);
    if (expectedPressSignalEmission)
        QCOMPARE(spyPress.count(), 1);
    else
        QCOMPARE(spyPress.count(), 0);

    subject->mouseMoveEvent(move);
    subject->mouseReleaseEvent(release);
    if (expectedReleaseSignalEmission)
        QCOMPARE(spyRelease.count(), 1);
    else
        QCOMPARE(spyRelease.count(), 0);

    subject->hide();
}

void Ut_WordRibbonItem::checkSize_data()
{
    QTest::addColumn<QString>("contentText");
    QTest::addColumn<QRect>("itemPaddingRect");
    QTest::addColumn<QRect>("itemContentRect");
    QTest::addColumn<QSize>("itemMinimumSize");
    QTest::addColumn<QSize>("itemPreferredSize");

    QTest::newRow("EmptyCondition")
            <<QString("") << QRect(0, 0, 46, 48) <<QRect(8, 5, 30, 38)
            <<QSize(46, 48) <<QSize(46, 48);

    QTest::newRow("OneWordCondition")
            <<QString("a") << QRect(0, 0, 46, 48) <<QRect(8, 5, 30, 38)
            <<QSize(46, 48) <<QSize(46, 48);
}

void Ut_WordRibbonItem::checkSize()
{
    qDebug() <<QTest::currentDataTag();
    QFETCH(QString, contentText);
    QFETCH(QRect, itemPaddingRect);
    QFETCH(QRect, itemContentRect);
    QFETCH(QSize, itemMinimumSize);
    QFETCH(QSize, itemPreferredSize);

    subject->setText(contentText);
    QCOMPARE(subject->paddingRect, itemPaddingRect);
    QCOMPARE(subject->contentRect, itemContentRect);
    QCOMPARE(subject->minimumSize, itemMinimumSize);
    QCOMPARE(subject->preferredSize, itemPreferredSize);
}

void Ut_WordRibbonItem::rotateToAngle(M::OrientationAngle angle)
{
    MPlainWindow::instance()->setOrientationAngle(angle);
    // wait until MSceneManager::orientationAngle() is updated.
    QTest::qWait(SceneRotationTime);
}


QTEST_APPLESS_MAIN(Ut_WordRibbonItem);


