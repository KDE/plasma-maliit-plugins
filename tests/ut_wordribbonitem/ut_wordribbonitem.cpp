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
    QTest::addColumn<QSize>("itemMinimumSize");
    QTest::addColumn<QSize>("itemPreferredSize");
    QTest::addColumn<QRect>("itemPaddingRect");
    QTest::addColumn<QRect>("itemContentRect");

    // Load WordRibbonItemStyle information from .css file.
    const MStyle * style = MTheme::style("WordRibbonItemStyle", QString("utWordRibbonItem"));

    const WordRibbonItemStyle * wordRibbonItemstyle =
            qobject_cast<const WordRibbonItemStyle *> (style);

    int cssPaddingLeft = wordRibbonItemstyle->paddingLeft();
    int cssPaddingRight = wordRibbonItemstyle->paddingRight();
    int cssPaddingTop = wordRibbonItemstyle->paddingTop();
    int cssPaddingBottom = wordRibbonItemstyle->paddingBottom();
    int cssMarginLeft = wordRibbonItemstyle->marginLeft();
    int cssMarginRight = wordRibbonItemstyle->marginRight();
    int cssMarginTop = wordRibbonItemstyle->marginTop();
    int cssMarginBottom = wordRibbonItemstyle->marginBottom();
    QSize cssMinimumSize = wordRibbonItemstyle->minimumSize();


    // Calculate size and rect for WordRibbonItem with empty string.
    QSize emptyConditionMinimumSize = cssMinimumSize;
    QSize emptyConditionPreferredSize = emptyConditionMinimumSize;

    QSizeF tmpSize = emptyConditionPreferredSize - QSize(cssMarginLeft + cssMarginRight,
                                                         cssMarginTop + cssMarginBottom);

    QRect emptyConditionPaddingRect = QRect(cssMarginLeft, cssMarginTop,
                                            tmpSize.width(), tmpSize.height());

    tmpSize -= QSizeF(cssPaddingLeft + cssPaddingRight,
                      cssPaddingTop + cssPaddingBottom);

    QRect emptyConditionContentRect = QRect(cssMarginLeft + cssPaddingLeft,
                                            cssMarginTop + cssPaddingTop,
                                            tmpSize.width(),
                                            tmpSize.height());

    QTest::newRow("EmptyCondition")
            <<QString("")
            <<emptyConditionMinimumSize <<emptyConditionPreferredSize
            <<emptyConditionPaddingRect <<emptyConditionContentRect;


    // Calculate size and rect for WordRibbonItem with one Character.
    QFontMetrics fm(wordRibbonItemstyle->font());
    QSize textSize = fm.size(Qt::TextSingleLine, QString::fromUtf8("我"));

    QSize oneCharacterMinimumSize = QSize(textSize.width() +
                                          cssPaddingLeft + cssPaddingRight +
                                          cssMarginLeft + cssMarginRight,
                                          textSize.height() +
                                          cssPaddingTop + cssPaddingBottom +
                                          cssMarginTop + cssMarginBottom);
    QSize oneCharacterPreferredSize = oneCharacterMinimumSize;

    tmpSize = oneCharacterPreferredSize - QSize(cssMarginLeft + cssMarginRight,
                                                cssMarginTop + cssMarginBottom);

    QRect oneCharacterPaddingRect = QRect(cssMarginLeft, cssMarginTop,
                                          tmpSize.width(), tmpSize.height());

    tmpSize -= QSizeF(cssPaddingLeft + cssPaddingRight,
                      cssPaddingTop + cssPaddingBottom);

    QRect oneCharacterContentRect = QRect(cssMarginLeft + cssPaddingLeft,
                                          cssMarginTop + cssPaddingTop,
                                          tmpSize.width(),
                                          tmpSize.height());

    QTest::newRow("OneWordCondition")
            <<QString::fromUtf8("我")
            <<oneCharacterMinimumSize <<oneCharacterPreferredSize
            <<oneCharacterPaddingRect <<oneCharacterContentRect;


    // Calculate size and rect for WordRibbonItem with two Characters.
    textSize = fm.size(Qt::TextSingleLine, QString::fromUtf8("我我"));

    QSize twoCharacterMinimumSize = QSize(textSize.width() +
                                          cssPaddingLeft + cssPaddingRight +
                                          cssMarginLeft + cssMarginRight,
                                          textSize.height() +
                                          cssPaddingTop + cssPaddingBottom +
                                          cssMarginTop + cssMarginBottom);
    QSize twoCharacterPreferredSize = twoCharacterMinimumSize;

    tmpSize = twoCharacterPreferredSize - QSize(cssMarginLeft + cssMarginRight,
                                                cssMarginTop + cssMarginBottom);

    QRect twoCharacterPaddingRect = QRect(cssMarginLeft, cssMarginTop,
                                          tmpSize.width(), tmpSize.height());

    tmpSize -= QSizeF(cssPaddingLeft + cssPaddingRight,
                      cssPaddingTop + cssPaddingBottom);

    QRect twoCharacterContentRect = QRect(cssMarginLeft + cssPaddingLeft,
                                          cssMarginTop + cssPaddingTop,
                                          tmpSize.width(),
                                          tmpSize.height());

    QTest::newRow("TwoWordCondition")
            <<QString::fromUtf8("我我")
            <<twoCharacterMinimumSize <<twoCharacterPreferredSize
            <<twoCharacterPaddingRect <<twoCharacterContentRect;

    MTheme::releaseStyle(style);
}

void Ut_WordRibbonItem::checkSize()
{
    qDebug() <<QTest::currentDataTag();
    QFETCH(QString, contentText);
    QFETCH(QSize, itemMinimumSize);
    QFETCH(QSize, itemPreferredSize);
    QFETCH(QRect, itemPaddingRect);
    QFETCH(QRect, itemContentRect);

    subject->setText(contentText);

    QCOMPARE(subject->minimumSize, itemMinimumSize);
    QCOMPARE(subject->preferredSize, itemPreferredSize);
    QCOMPARE(subject->paddingRect, itemPaddingRect);
    QCOMPARE(subject->contentRect, itemContentRect);
}


void Ut_WordRibbonItem::rotateToAngle(M::OrientationAngle angle)
{
    MPlainWindow::instance()->setOrientationAngle(angle);
    // wait until MSceneManager::orientationAngle() is updated.
    QTest::qWait(SceneRotationTime);
}


QTEST_APPLESS_MAIN(Ut_WordRibbonItem);


