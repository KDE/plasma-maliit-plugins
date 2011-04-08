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

//#define MIMABSTRACTKEYAREA_H

#include "ut_horizontalswitcher.h"
#include "horizontalswitcher.h"
#include "mimabstractkeyarea.h"
#include "utils.h"
#include "mimtestkeyarea.h"
#include "mimkeyarea.h"
#include <mplainwindow.h>

#include <MApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPointer>

Q_DECLARE_METATYPE(HorizontalSwitcher::SwitchDirection);
Q_DECLARE_METATYPE(QGraphicsWidget *);

void Ut_HorizontalSwitcher::initTestCase()
{
    static int argc = 2;
    static char *app_name[2] = { (char *) "ut_horizontalswitcher",
                                 (char *) "-software"};

    disableQtPlugins();
    app = new MApplication(argc, app_name);

    new MPlainWindow; // create singleton

    qRegisterMetaType<QGraphicsWidget *>();
}

void Ut_HorizontalSwitcher::cleanupTestCase()
{
    delete MPlainWindow::instance();
    delete app;
    app = 0;
}

void Ut_HorizontalSwitcher::init()
{
    subject = new HorizontalSwitcher(0);
}

void Ut_HorizontalSwitcher::cleanup()
{
    if (subject) {
        delete subject;
        subject = 0;
    }
}

void Ut_HorizontalSwitcher::testAddWidgets()
{
    QVERIFY(subject->count() == 0);
    QVERIFY(subject->current() == -1);
    QVERIFY(subject->currentWidget() == 0);

    const int numberOfWidgets = 5;

    for (int i = 0; i < numberOfWidgets; ++i) {
        QGraphicsWidget *w = 0;
        subject->addWidget(w = new QGraphicsWidget);
        QVERIFY(!w->isVisible() || w == subject->currentWidget());
        QVERIFY(w->parentItem() == subject);
        QVERIFY(w->scene() == subject->scene());
    }

    QVERIFY(subject->count() == numberOfWidgets);
    QVERIFY((subject->current() >= 0) && (subject->current() < subject->count()));
    QVERIFY(subject->currentWidget() != 0);
    QVERIFY(subject->currentWidget()->isVisible());

    subject->deleteAll();
    QVERIFY(subject->count() == 0);
    QVERIFY(subject->current() == -1);
}

void Ut_HorizontalSwitcher::testShowWidgets()
{
    const int numberOfWidgets = 3;
    QList<QGraphicsWidget *> widgets;
    QGraphicsWidget *w;

    for (int i = 0; i < numberOfWidgets; ++i) {
        w = new QGraphicsWidget;
        widgets.append(w);
        subject->addWidget(w);
    }

    subject->setCurrent(0);

    int visibleCount = 0;
    foreach(QGraphicsWidget * widget, widgets) {
        if (widget->isVisible())
            ++visibleCount;
    }

    QCOMPARE(visibleCount, 1);
}

void Ut_HorizontalSwitcher::testSetCurrent_data()
{
    QTest::addColumn<int>("numWidgets");
    QTest::addColumn<int>("current");
    QTest::addColumn<int>("expected");

    QTest::newRow("Empty1")     << 0 <<  0 << -1;
    QTest::newRow("Invalid0")   << 0 << -1 << -1;
    QTest::newRow("AddOne")     << 1 << -1 <<  0;
    QTest::newRow("AddTwo")     << 2 << -1 <<  0;
    QTest::newRow("3 widgets1") << 3 <<  0 <<  0;
    QTest::newRow("3 widgets2") << 3 <<  1 <<  1;
}

// FIXME: This test tests more about addWidget than it tests about setCurrent ...
void Ut_HorizontalSwitcher::testSetCurrent()
{
    QVERIFY(subject->count() == 0);

    QFETCH(int, numWidgets);
    QFETCH(int, current);
    QFETCH(int, expected);

    QGraphicsWidget *expectedWidget = 0;

    for (int i = 0; i < numWidgets; ++i) {
        subject->addWidget(new QGraphicsWidget);
    }

    if (expected >= 0) {
        expectedWidget = subject->widget(expected);
    }

    subject->setCurrent(current);
    QCOMPARE(subject->current(), expected);
    QCOMPARE(subject->currentWidget(), expectedWidget);
}

void Ut_HorizontalSwitcher::testSwitchLeftRight_data()
{
    QTest::addColumn<int>("widgetCount");
    QTest::addColumn<HorizontalSwitcher::SwitchDirection>("direction");
    QTest::addColumn<bool>("loop");
    QTest::addColumn<int>("currentIndex");
    QTest::addColumn<int>("expectedIndex");

    QTest::newRow("Left")            << 3 << HorizontalSwitcher::Left << false << 2 << 1;
    QTest::newRow("Left edge")       << 3 << HorizontalSwitcher::Left << false << 0 << 0;
    QTest::newRow("Left edge loop")  << 3 << HorizontalSwitcher::Left << true  << 0 << 2;

    QTest::newRow("Right")            << 3 << HorizontalSwitcher::Right << false << 1 << 2;
    QTest::newRow("Right edge")       << 3 << HorizontalSwitcher::Right << false << 2 << 2;
    QTest::newRow("Right edge loop")  << 3 << HorizontalSwitcher::Right << true  << 2 << 0;
}

void Ut_HorizontalSwitcher::testSwitchLeftRight()
{
    QFETCH(int, widgetCount);
    QFETCH(HorizontalSwitcher::SwitchDirection, direction);
    QFETCH(bool, loop);
    QFETCH(int, currentIndex);
    QFETCH(int, expectedIndex);

    QGraphicsWidget *expectedWidget = NULL;
    QList<QGraphicsWidget *> widgets;

    for (int i = 0; i < widgetCount; ++i) {
        // widgets freed in clean()
        QGraphicsWidget *widget = new QGraphicsWidget;
        widgets.append(widget);
        subject->addWidget(widget);
    }

    if (expectedIndex >= 0) {
        expectedWidget = widgets.at(expectedIndex);
    }

    subject->setLooping(loop);
    subject->setCurrent(currentIndex);

    subject->switchTo(direction);
    QCOMPARE(subject->current(), expectedIndex);
    QCOMPARE(subject->currentWidget(), expectedWidget);
}

void Ut_HorizontalSwitcher::testDisabledDuringTransitions_data()
{
    QTest::addColumn<bool>("playAnimation");
    QTest::newRow("without animation") << false;
    QTest::newRow("with animation") << true;
}

void Ut_HorizontalSwitcher::testDisabledDuringTransitions()
{
    QFETCH(bool, playAnimation);

    QGraphicsView view;
    view.setScene(new QGraphicsScene);
    view.show();

    QGraphicsWidget *root  = new QGraphicsWidget;
    QGraphicsWidget *left  = new QGraphicsWidget(root);
    QGraphicsWidget *right = new QGraphicsWidget(root);
    QSignalSpy spy(subject, SIGNAL(switchDone(QGraphicsWidget*, QGraphicsWidget*)));

    view.scene()->addItem(root);
    subject->setDuration(1);
    subject->setAnimationEnabled(playAnimation);
    subject->addWidget(left);
    subject->addWidget(right);

    QCOMPARE(subject->currentWidget(), left);
    QVERIFY(left->isEnabled());
    QVERIFY(right->isEnabled());

    subject->switchTo(HorizontalSwitcher::Right);
    waitForSignal(subject, SIGNAL(switchDone(QGraphicsWidget*, QGraphicsWidget*)), 1000);

    QCOMPARE(subject->currentWidget(), right);
    QVERIFY(left->isEnabled());
    QVERIFY(right->isEnabled());

    root->hide();
    root->show();
    QVERIFY(left->isEnabled());
    QVERIFY(right->isEnabled());
}

// comment below test cases due to MCompositor bug: NB#182701 breaks us
#if 0
void Ut_HorizontalSwitcher::testSwitchSignals()
{
    const int animationDuration = 10;

    QGraphicsWidget *leftWidget = new QGraphicsWidget;
    QGraphicsWidget *rightWidget = new QGraphicsWidget;

    subject->addWidget(leftWidget);
    subject->addWidget(rightWidget);

    subject->setCurrent(0); // left

    QSignalSpy spySwitchStartingIndex(subject, SIGNAL(switchStarting(int, int)));
    QSignalSpy spySwitchStartingWidget(subject, SIGNAL(switchStarting(QGraphicsWidget *, QGraphicsWidget *)));
    QSignalSpy spySwitchDoneIndex(subject, SIGNAL(switchDone(int, int)));
    QSignalSpy spySwitchDoneWidget(subject, SIGNAL(switchDone(QGraphicsWidget *, QGraphicsWidget *)));

    subject->setDuration(animationDuration);
    subject->switchTo(HorizontalSwitcher::Right);

    // Catch starting signals.
    QCOMPARE(spySwitchStartingIndex.count(), 1);
    QCOMPARE(spySwitchStartingWidget.count(), 1);
    QCOMPARE(spySwitchDoneIndex.count(), 0);
    QCOMPARE(spySwitchDoneWidget.count(), 0);

    QCOMPARE(spySwitchStartingIndex.at(0).at(0).value<int>(), 0);
    QCOMPARE(spySwitchStartingIndex.at(0).at(1).value<int>(), 1);
    QCOMPARE(spySwitchStartingWidget.at(0).at(0).value<QGraphicsWidget *>(), leftWidget);
    QCOMPARE(spySwitchStartingWidget.at(0).at(1).value<QGraphicsWidget *>(), rightWidget);

    QTest::qWait(animationDuration + 50);

    // Animation should be finished now.

    QCOMPARE(spySwitchStartingIndex.count(), 1);
    QCOMPARE(spySwitchStartingWidget.count(), 1);
    QCOMPARE(spySwitchDoneIndex.count(), 1);
    QCOMPARE(spySwitchDoneWidget.count(), 1);

    QCOMPARE(spySwitchDoneIndex.at(0).at(0).value<int>(), 0);
    QCOMPARE(spySwitchDoneIndex.at(0).at(1).value<int>(), 1);
    QCOMPARE(spySwitchDoneWidget.at(0).at(0).value<QGraphicsWidget *>(), leftWidget);
    QCOMPARE(spySwitchDoneWidget.at(0).at(1).value<QGraphicsWidget *>(), rightWidget);
}
#endif

void Ut_HorizontalSwitcher::testInitialSwitchTo()
{
    QCOMPARE(subject->current(), -1);
    subject->addWidget(new QGraphicsWidget);
    subject->setCurrent(0);
    QCOMPARE(subject->current(), 0);
    QVERIFY(subject->currentWidget()->isVisible());
}

void Ut_HorizontalSwitcher::testIsAtBoundary()
{
    QCOMPARE(subject->current(), -1);
    subject->addWidget(new QGraphicsWidget);

    subject->setCurrent(0);
    QVERIFY(subject->isAtBoundary(HorizontalSwitcher::Left) == true);
    QVERIFY(subject->isAtBoundary(HorizontalSwitcher::Right) == true);

    subject->addWidget(new QGraphicsWidget);
    subject->addWidget(new QGraphicsWidget);
    QVERIFY(subject->isAtBoundary(HorizontalSwitcher::Left) == true);
    QVERIFY(subject->isAtBoundary(HorizontalSwitcher::Right) == false);

    subject->setCurrent(1);
    QVERIFY(subject->isAtBoundary(HorizontalSwitcher::Left) == false);
    QVERIFY(subject->isAtBoundary(HorizontalSwitcher::Right) == false);

    subject->setCurrent(2);
    QVERIFY(subject->isAtBoundary(HorizontalSwitcher::Left) == false);
    QVERIFY(subject->isAtBoundary(HorizontalSwitcher::Right) == true);
}

void Ut_HorizontalSwitcher::testKeyOverrides()
{
    const int count = 5;
    QMap<QString, QSharedPointer<MKeyOverride> > overrides;

    LayoutData::SharedLayoutSection layouts[count];
    QPointer<MImTestKeyArea> keyAreas[count];

    for (int n = 0; n < count; ++n) {
        layouts[n] = LayoutData::SharedLayoutSection(new LayoutSection("meego"));
        keyAreas[n] = new MImTestKeyArea(layouts[n]); // will be deleted by HorizontalSwitcher
        subject->addWidget(keyAreas[n]);
    }

    subject->setKeyOverrides(overrides);

    for (int n = 0; n < count; ++n) {
        MImTestKeyArea *kba = qobject_cast<MImTestKeyArea *>(subject->widget(n));
        if (kba) {
            QCOMPARE(kba->setKeyOverridesCalls, 1);
            QCOMPARE(kba->setKeyOverridesParam, overrides);
        }
    }
}

void Ut_HorizontalSwitcher::testEnableHorizontalFlick_data()
{
    QTest::addColumn<bool>("enableSinglePageFlick");
    QTest::addColumn<int>("count");

    QTest::newRow("true 1")  << true  << 1;
    QTest::newRow("false 1") << false << 1;

    QTest::newRow("true 2")  << true  << 2;
    QTest::newRow("false 2") << false << 2;

    QTest::newRow("true 5")  << true  << 5;
    QTest::newRow("false 5") << false << 5;
}

void Ut_HorizontalSwitcher::testEnableHorizontalFlick()
{
    QFETCH(bool, enableSinglePageFlick);
    QFETCH(int, count);

    QMap<QString, QSharedPointer<MKeyOverride> > overrides;

    LayoutData::SharedLayoutSection layouts[count];
    QPointer<MImTestKeyArea> keyAreas[count];

    subject->enableSinglePageHorizontalFlick(enableSinglePageFlick);

    for (int n = 0; n < count; ++n) {
        layouts[n] = LayoutData::SharedLayoutSection(new LayoutSection("meego"));
        keyAreas[n] = new MImTestKeyArea(layouts[n]);
        subject->addWidget(keyAreas[n]);

        checkHorizontalFlickMode(enableSinglePageFlick);
    }

    subject->enableSinglePageHorizontalFlick(!enableSinglePageFlick);
    checkHorizontalFlickMode(!enableSinglePageFlick);

    subject->deleteAll();
    checkHorizontalFlickMode(!enableSinglePageFlick);

    subject->enableSinglePageHorizontalFlick(enableSinglePageFlick);
    checkHorizontalFlickMode(enableSinglePageFlick);
}

void Ut_HorizontalSwitcher::checkHorizontalFlickMode(bool expected)
{
    QCOMPARE(subject->m_enableSinglePageFlick, expected);

    for (int n = 0; n < subject->count(); ++n) {
        MImTestKeyArea *kba = qobject_cast<MImTestKeyArea *>(subject->widget(n));
        if (kba) {
            QCOMPARE(kba->allowedHorizontalFlick(), (expected || subject->count() > 1));
        }
    }
}

QTEST_APPLESS_MAIN(Ut_HorizontalSwitcher);
