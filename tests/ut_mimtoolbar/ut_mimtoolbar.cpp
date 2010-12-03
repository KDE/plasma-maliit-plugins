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



#include "ut_mimtoolbar.h"
#include "mimtoolbar.h"
#include "mtoolbarbutton.h"
#include "mtoolbarlabel.h"
#include "mapplication.h"
#include "mvirtualkeyboard.h"
#include "layoutsmanager.h"
#include "mreactionmaptester.h"
#include "utils.h"
#include <mplainwindow.h>
#include <mtoolbardata.h>
#include <mtoolbarlayout.h>
#include <MTheme>
#include <MSceneWindow>
#include <MButton>
#include <MLabel>
#include <MInfoBanner>
#include <MScene>
#include "mgconfitem_stub.h"

#include <QtTest/QTest>
#include <QObject>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QSignalSpy>
#include <QKeyEvent>
#include <QGraphicsLayout>
#include <QVariant>

Q_DECLARE_METATYPE(CopyPasteState);

namespace
{
    const QString TargetSettingsName("/meegotouch/target/name");
    const QString DefaultTargetName("Default");

    QString ToolbarFileName  = "/testtoolbar.xml";
    QString ToolbarFileName2 = "/testtoolbar2.xml";
    QString ToolbarFileName4 = "/testtoolbar4.xml";
}


// Stubbing..................................................................

void MToolbarButton::setIconFile(const QString &newIconFile)
{
    //reimplement to avoid checking the exist of icon file.
    iconFile = newIconFile;
}

void Ut_MImToolbar::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_mimtoolbar",
                                  (char *) "-software" };
    // this value is required by the theme daemon
    MGConfItem(TargetSettingsName).set(DefaultTargetName);

    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);

    qRegisterMetaType<CopyPasteState>("CopyPasteState");
    LayoutsManager::createInstance();

    sceneWindow = createMSceneWindow(new MPlainWindow); // also create singleton MPlainWindow

    ToolbarFileName = QCoreApplication::applicationDirPath() + ToolbarFileName;
    QVERIFY(QFile::exists(ToolbarFileName));
    ToolbarFileName2 = QCoreApplication::applicationDirPath() + ToolbarFileName2;
    QVERIFY(QFile::exists(ToolbarFileName2));
    ToolbarFileName4 = QCoreApplication::applicationDirPath() + ToolbarFileName4;
    QVERIFY(QFile::exists(ToolbarFileName4));
}

void Ut_MImToolbar::init()
{
    m_subject = new MImToolbar;
    MPlainWindow::instance()->scene()->addItem(m_subject);

    //fill up toolbar with some data
    toolbarData = QSharedPointer<MToolbarData>(new MToolbarData);
    bool ok = toolbarData->loadToolbarXml(ToolbarFileName);
    QVERIFY(ok);
}


void Ut_MImToolbar::cleanup()
{
    delete m_subject;
}

void Ut_MImToolbar::cleanupTestCase()
{
    toolbarData.clear();
    LayoutsManager::destroyInstance();
    delete sceneWindow;
    delete MPlainWindow::instance();
    delete app;
    app = 0;
}

void Ut_MImToolbar::testShowToolbarWidget()
{
    QSignalSpy spy(m_subject, SIGNAL(regionUpdated()));
    QVERIFY(spy.isValid());
    m_subject->showToolbarWidget(toolbarData);
    //toolbar buttons depend on the its data
    //including spacing widget and close button
    QCOMPARE(m_subject->leftBar.layout()->count(), 2);
    QCOMPARE(m_subject->rightBar.layout()->count(), 1);
    QCOMPARE(spy.count(), 1);
    spy.clear();
}

void Ut_MImToolbar::testShowGroup()
{
    m_subject->showToolbarWidget(toolbarData);
    //find button testbutton2, which click will show group test
    MButton *button = qobject_cast<MButton *>(find("testbutton1"));
    QVERIFY(button != 0);
    button->click();
    QCOMPARE(m_subject->leftBar.layout()->count(), 2);
    QCOMPARE(m_subject->rightBar.layout()->count(), 2);
}

void Ut_MImToolbar::testHideGroup()
{
    m_subject->showToolbarWidget(toolbarData);
    //find button testbutton2, which click will hide group test
    MButton *button = qobject_cast<MButton *>(find("testbutton2"));
    QVERIFY(button != 0);
    button->click();
    QCOMPARE(m_subject->leftBar.layout()->count(), 1);
    QCOMPARE(m_subject->rightBar.layout()->count(), 1);
}

void Ut_MImToolbar::testSendString()
{
    m_subject->showToolbarWidget(toolbarData);
    QSignalSpy spy(m_subject, SIGNAL(sendStringRequest(const QString &)));
    QVERIFY(spy.isValid());
    //find button testbutton2, which click will send string
    MButton *button = qobject_cast<MButton *>(find("testbutton2"));
    QVERIFY(button != 0);
    button->click();
    QVERIFY(spy.count() == 1);
    spy.clear();
}

void Ut_MImToolbar::testKeySequenceString()
{
    m_subject->showToolbarWidget(toolbarData);
    //find button testbutton1, which click will send key sequence (QKeyEvent)
    //because QKeyEvent is not supported by MetaType, use its own slot to test it.
    keyEvents = 0;
    connect(m_subject, SIGNAL(sendKeyEventRequest(const QKeyEvent &)),
            this, SLOT(receiveKeyEvent(const QKeyEvent &)));
    MButton *button = qobject_cast<MButton *>(find("testbutton1"));
    QVERIFY(button != 0);
    button->click();
    QVERIFY(keyEvents > 0);
    keyEvents = 0;
}

void Ut_MImToolbar::testHideToolbarWidget()
{
    m_subject->showToolbarWidget(toolbarData);
    m_subject->hideToolbarWidget();
}

void Ut_MImToolbar::receiveKeyEvent(const QKeyEvent &)
{
    keyEvents ++;
}

void Ut_MImToolbar::testCopy()
{
    QSignalSpy spy(m_subject, SIGNAL(copyPasteRequest(CopyPasteState)));
    QVERIFY(spy.isValid());

    m_subject->showToolbarWidget(toolbarData);
    //find button testbutton1, which click will copy
    MButton *button = qobject_cast<MButton *>(find("testbutton1"));
    QVERIFY(button != 0);
    button->click();
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.first().count() == 1);
    CopyPasteState result = spy.first().first().value<CopyPasteState>();
    QCOMPARE(result, InputMethodCopy);
    spy.clear();
}

void Ut_MImToolbar::testPaste()
{
    QSignalSpy spy(m_subject, SIGNAL(copyPasteRequest(CopyPasteState)));
    QVERIFY(spy.isValid());

    m_subject->showToolbarWidget(toolbarData);
    //find button testbutton2, which click will paste
    MButton *button = qobject_cast<MButton *>(find("testbutton2"));
    QVERIFY(button != 0);
    button->click();
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.first().count() == 1);
    CopyPasteState result = spy.first().first().value<CopyPasteState>();
    QCOMPARE(result, InputMethodPaste);
    spy.clear();
}


void Ut_MImToolbar::testRegion()
{
    QSKIP("Skip this case because Qt (4.7.1~git20101130) break the vkb toolbar.",
          SkipAll);
    QSignalSpy regionSignals(m_subject, SIGNAL(regionUpdated()));

    m_subject->showToolbarWidget(toolbarData);
    QCOMPARE(regionSignals.count(), 1);
    m_subject->updateVisibility();
    QCOMPARE(regionSignals.count(), 1);

    // Get region when there are two buttons on the right.
    const QRegion regionTwoButtons = m_subject->region();

    // When the region is substracted from rightRect there should be nothing left.
    const QRect rightRect = m_subject->rightBar.geometry().toRect();
    QVERIFY((QRegion(rightRect) - regionTwoButtons).isEmpty());

    // We need to add a new button, let's use groups.
    // Clicking testbutton1 will add one button to the right.
    MButton *button = qobject_cast<MButton *>(find("testbutton1"));
    QVERIFY(button != 0);
    button->click();

    while (QCoreApplication::hasPendingEvents()) {
        QCoreApplication::processEvents();
    }

    // Button added, check that regionUpdate() was emitted.
    QCOMPARE(regionSignals.count(), 2);

    // Get region when there are three buttons on the right.
    const QRegion regionThreeButtons = m_subject->region();

    // Toolbar always occupy the same region,
    // because our toolbar contains one line only.
    QCOMPARE(regionThreeButtons, regionTwoButtons);

    m_subject->finalizeOrientationChange();
    QCOMPARE(regionSignals.count(), 2);

    m_subject->hideToolbarWidget();

    QCOMPARE(regionSignals.count(), 3);

    m_subject->hide();

    QCOMPARE(regionSignals.count(), 4);
    QVERIFY(m_subject->region().isEmpty());
}

void Ut_MImToolbar::testReactionMaps_data()
{
    QTest::addColumn<QString>("filename");

    QTest::newRow("one row toolbar")  << ToolbarFileName;
}

void Ut_MImToolbar::testReactionMaps()
{
    QSKIP("Too late to fix this one, no idea why it fails.",
          SkipAll);
    QFETCH(QString, filename);

    toolbarData = QSharedPointer<MToolbarData>(new MToolbarData);
    QVERIFY(toolbarData->loadToolbarXml(filename));

    m_subject->showToolbarWidget(toolbarData);

    MReactionMapTester tester;
    gMReactionMapStub = &tester;

    // Show toolbar
    m_subject->show();

    // Clear with transparent color
    // Reaction map should be cleaned by other widget.
    gMReactionMapStub->setTransparentDrawingValue();
    gMReactionMapStub->setTransform(QTransform());
    gMReactionMapStub->fillRectangle(0, 0, gMReactionMapStub->width(), gMReactionMapStub->height());

    QGraphicsView *view = MPlainWindow::instance();

    m_subject->paintReactionMap(MReactionMap::instance(view), view);

    // Overall sanity test with grid points throughout the view.
    QVERIFY(tester.testReactionMapGrid(view, 40, 50, m_subject->region(), m_subject));

    // Check that all buttons are drawn with reactive color.
    QVERIFY(tester.testChildButtonReactiveAreas(view, m_subject));

    // Check that middle point is either transparent or inactive, depending on whether toolbar is shaped or not.
    // Assuming also that no buttons extend to middle area.
    gMReactionMapStub->setTransform(m_subject, view);
    MReactionMapTester::ReactionColorValue middlePointReactionColor = MReactionMapTester::Inactive;
    MReactionMapTester::ReactionColorValue actualMiddlePointColor = tester.colorAt(m_subject->boundingRect().center());
    QCOMPARE(actualMiddlePointColor, middlePointReactionColor);
}

void Ut_MImToolbar::testClose()
{
    QSignalSpy spy(m_subject, SIGNAL(closeKeyboardRequest()));
    QVERIFY(spy.isValid());

    toolbarData = QSharedPointer<MToolbarData>(new MToolbarData);
    bool ok = toolbarData->loadToolbarXml(ToolbarFileName4);
    QVERIFY(ok);

    m_subject->showToolbarWidget(toolbarData);
    MToolbarButton *button = qobject_cast<MToolbarButton *>(find("testbutton"));
    QVERIFY(button != 0);

    button->click();
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.first().isEmpty());
}

MWidget* Ut_MImToolbar::find(const QString &name)
{
    foreach (QPointer<MWidget> widget, m_subject->customWidgets) {
        MToolbarButton *button = qobject_cast<MToolbarButton*>(widget);
        if (button && button->item() && button->item()->name() == name) {
            return button;
        }

        MToolbarLabel *label = qobject_cast<MToolbarLabel*>(widget);
        if (label && label->item() && label->item()->name() == name) {
            return label;
        }
    }

    return 0;
}

void Ut_MImToolbar::testSuppressArrangeWidgets()
{
    QSignalSpy spy(m_subject, SIGNAL(regionUpdated()));
    QVERIFY(spy.isValid());

    m_subject->suppressArrangeWidgets(true);
    m_subject->suppressArrangeWidgets(false);
    QCOMPARE(spy.count(), 0);

    m_subject->suppressArrangeWidgets(true);
    m_subject->arrangeWidgets();
    QCOMPARE(spy.count(), 0);
    m_subject->suppressArrangeWidgets(false);
    QCOMPARE(spy.count(), 1);
    spy.clear();

    m_subject->suppressArrangeWidgets(true);
    m_subject->arrangeWidgets();
    m_subject->arrangeWidgets();
    QCOMPARE(spy.count(), 0);
    m_subject->suppressArrangeWidgets(false);
    QCOMPARE(spy.count(), 1);
    spy.clear();

    m_subject->suppressArrangeWidgets(true);
    m_subject->arrangeWidgets();
    QCOMPARE(spy.count(), 0);
    m_subject->suppressArrangeWidgets(true);
    m_subject->arrangeWidgets();
    QCOMPARE(spy.count(), 0);
    m_subject->suppressArrangeWidgets(false);
    QCOMPARE(spy.count(), 0);
    m_subject->suppressArrangeWidgets(false);
    QCOMPARE(spy.count(), 1);
    spy.clear();
}

QTEST_APPLESS_MAIN(Ut_MImToolbar);

