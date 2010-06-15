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
#include "mvirtualkeyboardstyle.h"
#include <mplainwindow.h>
#include <mtoolbardata.h>
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
    QString ToolbarFileName = "/testtoolbar.xml";

    int indexOf(const QGraphicsLayout *layout, const QGraphicsLayoutItem *item)
    {
        int result = -1;

        for (int n = 0; n < layout->count(); ++n) {
            if (layout->itemAt(n) == item) {
                result = n;
                break;
            }
        }

        return result;
    }
}

void MToolbarButton::setIconFile(const QString &newIconFile)
{
    //reimplement to avoid checking the exist of icon file.
    iconFile = newIconFile;
}

void Ut_MImToolbar::initTestCase()
{
    static int dummyArgc = 1;
    static char *dummyArgv[1] = { (char *) "./ut_mimtoolbar" };
    // Avoid waiting if im server is not responding
    MApplication::setLoadMInputContext(false);
    app = new MApplication(dummyArgc, dummyArgv);

    qRegisterMetaType<CopyPasteState>("CopyPasteState");
    LayoutsManager::createInstance();

    QString cssFile("../../m-keyboard/theme/864x480.css");
    if (!QFile::exists(cssFile)) {
        cssFile = "/usr/share/meegotouch/virtual-keyboard/css/864x480.css";
        QVERIFY(QFile::exists(cssFile));
    }
    MTheme::instance()->loadCSS(cssFile);

    style = new MVirtualKeyboardStyleContainer;
    style->initialize("MVirtualKeyboard", "MVirtualKeyboardView", 0);

    new MPlainWindow; // Create singleton

    ToolbarFileName = QCoreApplication::applicationDirPath() + ToolbarFileName;
    QVERIFY(QFile::exists(ToolbarFileName));
}

void Ut_MImToolbar::init()
{
    m_subject = new MImToolbar(*style);
    MPlainWindow::instance()->scene()->addItem(m_subject);

    //fill up toolbar with some data
    toolbarData = QSharedPointer<MToolbarData>(new MToolbarData);
    bool ok = toolbarData->loadNokiaToolbarXml(ToolbarFileName);
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
    delete MPlainWindow::instance();
    delete style;
    style = 0;
    delete app;
    app = 0;
}

void Ut_MImToolbar::testCopyPasteButton()
{
    QList<bool> copyAvailable;
    QList<bool> pasteAvailable;
    QList<bool> visible;
    QList<QString> label;
    QList<int> expectedSignal;
    QList<CopyPasteState> signalParameter;

    MButton *button = m_subject->copyPaste;
    QSignalSpy spy(m_subject, SIGNAL(copyPasteClicked(CopyPasteState)));

    QVERIFY(button != 0);
    QVERIFY(spy.isValid());

    copyAvailable << false << false << true << true;
    pasteAvailable << false << true << false << true;
    QVERIFY(copyAvailable.count() == pasteAvailable.count());

    visible << false << true << true << true;
    QVERIFY(copyAvailable.count() == visible.count());

    label << "" << "Paste" << "Copy" << "Copy";
    QVERIFY(copyAvailable.count() == label.count());

    expectedSignal << 0 << 1 << 1 << 1;
    QVERIFY(copyAvailable.count() == expectedSignal.count());

    signalParameter << InputMethodNoCopyPaste << InputMethodPaste
                    << InputMethodCopy << InputMethodCopy;


    for (int n = 0; n < copyAvailable.count(); ++n) {
        qDebug() << "test step" << n;

        m_subject->setCopyPasteButton(copyAvailable.at(n), pasteAvailable.at(n));

        QVERIFY(button->isVisible() == visible.at(n));
        if (button->isVisible()) {
            QVERIFY(indexOf(m_subject->rightBar.layout(), button) >= 0);
        } else {
            QVERIFY(indexOf(m_subject->rightBar.layout(), button) == -1);
        }
        QVERIFY(button->text().contains(label.at(n), Qt::CaseInsensitive));

        m_subject->copyPasteButtonHandler();
        QVERIFY(spy.count() == expectedSignal.at(n));
        if (spy.count()) {
            QVERIFY(spy.first().count() == 1);
            CopyPasteState result = spy.first().first().value<CopyPasteState>();
            QCOMPARE(result, signalParameter.at(n));
        }
        spy.clear();
    }
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
    //find button testbutton2, which click will copy
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
    QSignalSpy regionSignals(m_subject, SIGNAL(regionUpdated()));

    m_subject->showToolbarWidget(toolbarData);
    QCOMPARE(regionSignals.count(), 1);
    m_subject->updateVisibility();
    QCOMPARE(regionSignals.count(), 2);

    // Get region when there are two buttons on the right.
    QRegion regionTwoButtons = m_subject->region();

    // When the region is substracted from rightRect there should be nothing left.
    QRect rightRect = m_subject->rightBar.geometry().toRect();
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
    QCOMPARE(regionSignals.count(), 3);

    // Get region when there are three buttons on the right.
    QRegion regionThreeButtons = m_subject->region();

    // Old region (two buttons) is not enough to cover rightRect
    // but the new region is.
    rightRect = m_subject->rightBar.geometry().toRect();
    QVERIFY(!(QRegion(rightRect) - regionTwoButtons).isEmpty());
    QVERIFY((QRegion(rightRect) - regionThreeButtons).isEmpty());

    m_subject->hideToolbarWidget();

    QCOMPARE(regionSignals.count(), 4);

    m_subject->hide();

    QCOMPARE(regionSignals.count(), 5);
    QVERIFY(m_subject->region().isEmpty());
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

QTEST_APPLESS_MAIN(Ut_MImToolbar);

