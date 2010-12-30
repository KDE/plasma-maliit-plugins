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



#include "mreactionmaptester.h"

#include "mgconfitem_stub.h"
#include "mvirtualkeyboard.h"
#include "mvirtualkeyboardstyle.h"
#include "horizontalswitcher.h"
#include "keyevent.h"
#include "mimabstractkeyarea.h"
#include "layoutsmanager.h"
#include "symbolview.h"
#include "ut_symbolview.h"
#include "mplainwindow.h"
#include "utils.h"

#include <MApplication>
#include <MButton>
#include <MScene>
#include <MSceneManager>
#include <MSceneWindow>
#include <MTheme>

#include <QDir>
#include <QGraphicsLinearLayout>
#include <QSignalSpy>

namespace
{
    const QString InputMethodSettingName("/meegotouch/inputmethods/virtualkeyboard/layouts");
    const QString DefaultLayoutSettingName("/meegotouch/inputmethods/virtualkeyboard/layouts/default");
    const QString DefaultLayout("en_us.xml");

    const QString TargetSettingsName("/meegotouch/target/name");
    const QString DefaultTargetName("Default");

    const int SceneRotationTime = 1400;
} // namespace

Q_DECLARE_METATYPE(QList<QPoint>)
Q_DECLARE_METATYPE(HorizontalSwitcher::SwitchDirection)

void Ut_SymbolView::initTestCase()
{
    static int argc = 2;
    static char *app_name[2] = { (char *) "ut_symbolview",
                                 (char *) "-software" };

    disableQtPlugins();

    MGConfItem target(TargetSettingsName);
    target.set(DefaultTargetName); // this value is required by the theme daemon

    app = new MApplication(argc, app_name);
    style = new MVirtualKeyboardStyleContainer;
    style->initialize("MVirtualKeyboard", "MVirtualKeyboardView", 0);

    MGConfItem inputMethodSetting(InputMethodSettingName);

    QStringList langlist;
    langlist << "en_us.xml";
    inputMethodSetting.set(QVariant(langlist));

    MGConfItem defaultLayoutSetting(DefaultLayoutSettingName);
    defaultLayoutSetting.set(QVariant(DefaultLayout));

    LayoutsManager::createInstance();

    qRegisterMetaType<KeyEvent>("KeyEvent");

    new MPlainWindow;

    if ((*style)->paddingTop() <= 0) {
        QSKIP("This test is sipped due to incorrect value received from CSS", SkipAll);
    }
}

void Ut_SymbolView::cleanupTestCase()
{
    delete MPlainWindow::instance();
    LayoutsManager::destroyInstance();
    delete style;
    style = 0;
    delete app;
    app = 0;
}

void Ut_SymbolView::init()
{
    subject = new SymbolView(LayoutsManager::instance(), style, DefaultLayout);

    // Add to scene so reaction maps are drawn.
    // SymView needs scene window as parent so positions itself correctly
    // in the scene in portrait mode.
    parent = new MSceneWindow;
    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(parent);
    subject->setParentItem(parent);
}

void Ut_SymbolView::cleanup()
{
    // Make sure default stub is restored.
    gMReactionMapStub = &gDefaultMReactionMapStub;

    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(parent);
    subject->setParentItem(0);
    delete parent;
    delete subject;
    subject = 0;
}

void Ut_SymbolView::testReactiveButtonAreas_data()
{
    QTest::addColumn<int>("orientationAngle");

    // The following locations are supplementary to reactionmaptester's tests
    // and only required to check special cases.
    QTest::addColumn< QList<QPoint> >("transparentLocations");

    QList<QPoint> transparentLocationsLandscape;
    transparentLocationsLandscape
        << QPoint(1, -10) // transparent above symbol view
        << QPoint(600, -10); // transparent above symbol view

    QList<QPoint> transparentLocationsPortrait;
    transparentLocationsPortrait
        << QPoint(1, -10) // transparent above symbol view
        << QPoint(400, -10); // transparent above symbol view

    QTest::newRow("Angle 0") << 0 << transparentLocationsLandscape;
    QTest::newRow("Angle 90") << 90 << transparentLocationsPortrait;
}

void Ut_SymbolView::testReactiveButtonAreas()
{
    QFETCH(int, orientationAngle);
    QFETCH(QList<QPoint>, transparentLocations);

    rotateToAngle(static_cast<M::OrientationAngle>(orientationAngle));

    MReactionMapTester tester;
    gMReactionMapStub = &tester;

    QGraphicsView *view = MPlainWindow::instance();

    // Clear with transparent color
    gMReactionMapStub->setTransparentDrawingValue();
    gMReactionMapStub->setTransform(QTransform());
    gMReactionMapStub->fillRectangle(0, 0, gMReactionMapStub->width(), gMReactionMapStub->height());

    subject->showSymbolView();

    subject->paintReactionMap(MReactionMap::instance(view), view);

    // Check that all buttons are covered by reactive area
    QVERIFY(tester.testChildButtonReactiveAreas(view, subject));

    // Test the next tab also.
    subject->switchToNextPage();
    QTest::qWait(600);
    QVERIFY(!subject->pageSwitcher->isRunning());
    // After the switch the reactive areas should be updated.

    // Check that all buttons are covered by reactive area
    QVERIFY(tester.testChildButtonReactiveAreas(view, subject));

    // Following coordinates will be given in subject coordinates.
    gMReactionMapStub->setTransform(subject, view);

    foreach(const QPoint & pos, transparentLocations) {
        QCOMPARE(tester.colorAt(pos), MReactionMapTester::Transparent);
    }

    // Check locations that should be inactive. This test is really only for
    // testing that at least something is drawn inactive, and not only reactive
    // areas are drawn.
    const QRectF br(subject->boundingRect());
    QList<QPoint> inactiveLocations;
    inactiveLocations
            << QPoint(br.left(), br.center().y()) // Left margin
            << QPoint(br.right()-1, br.center().y()); // Right margin

    foreach(const QPointF & pos, inactiveLocations) {
        QCOMPARE(tester.colorAt(pos), MReactionMapTester::Inactive);
    }
}

void Ut_SymbolView::testReactiveWholeScreen_data()
{
    QTest::addColumn<int>("orientationAngle");

    QTest::newRow("Angle 0") << 0;
    QTest::newRow("Angle 90") << 90;
    QTest::newRow("Angle 180") << 180;
    QTest::newRow("Angle 270") << 270;
}

void Ut_SymbolView::testReactiveWholeScreen()
{
    QFETCH(int, orientationAngle);

    rotateToAngle(static_cast<M::OrientationAngle>(orientationAngle));

    MReactionMapTester tester;
    gMReactionMapStub = &tester;
    // Clear with transparent
    gMReactionMapStub->setTransparentDrawingValue();
    gMReactionMapStub->setTransform(QTransform());
    gMReactionMapStub->fillRectangle(0, 0, gMReactionMapStub->width(), gMReactionMapStub->height());

    subject->showSymbolView();

    subject->paintReactionMap(MReactionMap::instance(MPlainWindow::instance()), MPlainWindow::instance());

    const bool gridpass = tester.testReactionMapGrid(
                              MPlainWindow::instance(),
                              20, 50, subject->interactiveRegion(),
                              subject);

    QVERIFY(gridpass);

    gMReactionMapStub = &gDefaultMReactionMapStub;
}


void Ut_SymbolView::testOthers()
{
    //at least we should not crash
    subject->organizeContent();

    subject->hideSymbolView();
    subject->showSymbolView();
}

void Ut_SymbolView::testChangeToOpenMode()
{
    subject->showSymbolView();
    QVERIFY(subject->isActive() == true);
    subject->hideSymbolView();
    QVERIFY(subject->isActive() == false);
}

void Ut_SymbolView::testChangeTab_data()
{
    QTest::addColumn<int>("initialTabIndex");
    QTest::addColumn<HorizontalSwitcher::SwitchDirection>("direction");
    QTest::addColumn<int>("finalTabIndex"); // expected

    QTest::newRow("Next tab")   << 0 << HorizontalSwitcher::Right << 1;
    QTest::newRow("Prev tab")   << 1 << HorizontalSwitcher::Left << 0;
}

void Ut_SymbolView::testChangeTab()
{
    QFETCH(int, initialTabIndex);
    QFETCH(HorizontalSwitcher::SwitchDirection, direction);
    QFETCH(int, finalTabIndex);

    subject->pageSwitcher->setDuration(0);
    subject->pageSwitcher->setCurrent(initialTabIndex);

    QCOMPARE(subject->activePage, subject->pageSwitcher->current());

    if (direction == HorizontalSwitcher::Right) {
        subject->switchToNextPage();
    } else if (direction == HorizontalSwitcher::Left){
        subject->switchToPrevPage();
    }

    QCOMPARE(subject->activePage, finalTabIndex);
}

void Ut_SymbolView::testHideWithFlick_data()
{
    QTest::addColumn<int>("tabIndex");
    QTest::newRow("Sym tab") << 0;
    QTest::newRow("Ace tab") << 1;
}


void Ut_SymbolView::testHideWithFlick()
{
    QFETCH(int, tabIndex);
    QVERIFY(subject->pageSwitcher && subject->pageSwitcher->count() >= tabIndex);
    MImAbstractKeyArea *page = static_cast<MImAbstractKeyArea *>(subject->pageSwitcher->widget(tabIndex));

    // Using qtest rows we dont need to worry about symbol view's show/hide timers
    // ignoring the call because subject is re-created every time.
    subject->showSymbolView();
    QCOMPARE(subject->isActive(), true);
    emit page->flickDown();
    QCOMPARE(subject->isActive(), false);
}


void Ut_SymbolView::testSetLayout()
{
    // set an invalid layout, at least it won't crash
    // and still has the laste valid layout.
    QString oldLayout = subject->currentLayout;
    subject->setLayout("ThisLayoutShouldNotExist");
    QVERIFY(subject->currentLayout != "ThisLayoutShouldNotExist");
    QCOMPARE(subject->currentLayout, oldLayout);
}

void Ut_SymbolView::testHardwareState()
{
    subject->setKeyboardState(MInputMethod::OnScreen);
    QCOMPARE(subject->activeState, MInputMethod::OnScreen);

    // Make sure were in landscape mode when in hardware state
    rotateToAngle(M::Angle0);

    subject->setKeyboardState(MInputMethod::Hardware);
    QCOMPARE(subject->activeState, MInputMethod::Hardware);
}

void Ut_SymbolView::testSetTemporarilyHidden()
{
    QCOMPARE(subject->activity, SymbolView::Inactive);

    subject->setTemporarilyHidden(true);
    QCOMPARE(subject->activity, SymbolView::Inactive);

    subject->setTemporarilyHidden(false);
    QCOMPARE(subject->activity, SymbolView::Inactive);

    subject->showSymbolView();

    QTest::qWait(200);

    QCOMPARE(subject->activity, SymbolView::Active);

    subject->setTemporarilyHidden(false);
    QCOMPARE(subject->activity, SymbolView::Active);

    subject->setTemporarilyHidden(true);
    QCOMPARE(subject->activity, SymbolView::TemporarilyInactive);

    QTest::qWait(200);

    subject->setTemporarilyHidden(false);
    QCOMPARE(subject->activity, SymbolView::Active);
}

void Ut_SymbolView::rotateToAngle(M::OrientationAngle angle)
{
    subject->prepareToOrientationChange();
    MPlainWindow::instance()->setOrientationAngle(angle);
    QTest::qWait(SceneRotationTime);// wait until MSceneManager::orientationAngle() is updated.
    subject->finalizeOrientationChange();
}

QTEST_APPLESS_MAIN(Ut_SymbolView);
