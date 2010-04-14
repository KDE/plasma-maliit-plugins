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



#include "ut_duiimtoolbar.h"
#include "duiimtoolbar.h"
#include "toolbarmanager.h"
#include "toolbardata.h"
#include "duiapplication.h"
#include "duivirtualkeyboard.h"
#include "layoutsmanager.h"
#include "duivirtualkeyboardstyle.h"
#include <duiplainwindow.h>
#include <DuiTheme>
#include <DuiSceneWindow>
#include <DuiButton>
#include <DuiInfoBanner>
#include "duigconfitem_stub.h"

#include <QtTest/QTest>
#include <QObject>
#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QSignalSpy>
#include <QKeyEvent>
#include <QGraphicsLayout>
#include <QVariant>

Q_DECLARE_METATYPE(CopyPasteState);
namespace
{
    // This file doesn't really exist in filesystem, its memory representation
    // is constructed in init().
    QString toolbarFileName("/usr/share/dui/imtoolbars/testtoolbar.xml");
    //! indicator label for latin
    const QString LatinShiftOffIndicatorLabel("abc");
    const QString LatinShiftOnIndicatorLabel("Abc");
    const QString LatinShiftLockedIndicatorLabel("ABC");
    //! indicator label for cyrillic
    const QString CyrillicShiftOffIndicatorLabel = QString("%1%2%3")
            .arg(QChar(0x0430))
            .arg(QChar(0x0431))
            .arg(QChar(0x0432));
    const QString CyrillicShiftOnIndicatorLabel = QString("%1%2%3")
            .arg(QChar(0x0410))
            .arg(QChar(0x0431))
            .arg(QChar(0x0432));
    const QString CyrillicShiftLockedIndicatorLabel = QString("%1%2%3")
            .arg(QChar(0x0410))
            .arg(QChar(0x0411))
            .arg(QChar(0x0412));
    //! indicator label for arabic
    const QString ArabicIndicatorLabel = QString("%1%2%3%4%5")
                                         .arg(QChar(0x0627))
                                         .arg(QChar(0x200C))
                                         .arg(QChar(0x0628))
                                         .arg(QChar(0x200C))
                                         .arg(QChar(0x062A));
    //! indicator label for FN
    const QString FNOnIndicatorLabel("123");
    const QString FNLockedIndicatorLabel("<U>123</U>");
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;
    const QString SystemDisplayLanguage("/Dui/i18n/Language");

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
    int hideLockOnInfoBannerCalls = 0;
}
void DuiImToolbar::hideLockOnInfoBanner()
{
    //reimplement hideLockOnInfoBanner to avoid to show/hide infobanner.
    ++hideLockOnInfoBannerCalls;
}

void Ut_DuiImToolbar::initTestCase()
{
    static int dummyArgc = 1;
    static char *dummyArgv[1] = { (char *) "./ut_duiimtoolbar" };
    // Avoid waiting if im server is not responding
    DuiApplication::setLoadDuiInputContext(false);
    app = new DuiApplication(dummyArgc, dummyArgv);

    qRegisterMetaType<CopyPasteState>("CopyPasteState");
    LayoutsManager::createInstance();

    QString cssFile("../../dui-keyboard/theme/864x480.css");
    if (!QFile::exists(cssFile)) {
        cssFile = "/usr/share/dui/virtual-keyboard/css/864x480.css";
        QVERIFY(QFile::exists(cssFile));
    }
    DuiTheme::instance()->loadCSS(cssFile);

    style = new DuiVirtualKeyboardStyleContainer;
    style->initialize("DuiVirtualKeyboard", "DuiVirtualKeyboardView", 0);
}


void Ut_DuiImToolbar::init()
{
    m_subject = new DuiImToolbar(*style);

    //fill up toolbar with some data
    ToolbarData *toolbar = new ToolbarData;
    toolbar->toolbarFileName = toolbarFileName;
    ToolbarButton *b1 = new ToolbarButton();
    b1->name = "test1";
    b1->group = "group1";
    b1->priority = 0;
    b1->orientation = Dui::Landscape;
    b1->showOn = ToolbarButton::Always;
    b1->hideOn = ToolbarButton::Undefined;
    b1->alignment = Qt::AlignLeft;
    b1->text = "test1";
    b1->textId = "";
    ToolbarButton::Action *action11 = new ToolbarButton::Action(ToolbarButton::SendKeySequence);
    action11->keys = "Ctrl+I";
    b1->actions.append(action11);
    ToolbarButton::Action *action12 = new ToolbarButton::Action(ToolbarButton::Copy);
    b1->actions.append(action12);
    ToolbarButton::Action *action13 = new ToolbarButton::Action(ToolbarButton::ShowGroup);
    action13->group = "group3";
    b1->actions.append(action13);
    toolbar->buttons.append(b1);

    ToolbarButton *b2 = new ToolbarButton();
    b2->name = "test2";
    b2->group = "group2";
    b2->priority = 0;
    b2->orientation = Dui::Landscape;
    b2->showOn = ToolbarButton::Always;
    b2->hideOn = ToolbarButton::Undefined;
    b2->alignment = Qt::AlignRight;
    b2->text = "test2";
    b2->textId = "";
    ToolbarButton::Action *action21 = new ToolbarButton::Action(ToolbarButton::SendString);
    action21->text = "test string";
    b2->actions.append(action21);
    ToolbarButton::Action *action22 = new ToolbarButton::Action(ToolbarButton::Paste);
    b2->actions.append(action22);
    ToolbarButton::Action *action23 = new ToolbarButton::Action(ToolbarButton::HideGroup);
    action23->group = "group1";
    b2->actions.append(action23);
    toolbar->buttons.append(b2);

    ToolbarButton *b3 = new ToolbarButton();
    b3->name = "test3";
    b3->group = "group3";
    b3->priority = 1;
    b3->orientation = Dui::Landscape;
    b3->showOn = ToolbarButton::WhenSelectingText;
    b3->hideOn = ToolbarButton::Undefined;
    b3->alignment = Qt::AlignRight;
    b3->text = "test3";
    b3->textId = "";
    toolbar->buttons.append(b3);
    m_subject->toolbarMgr->toolbars.append(toolbar);
}


void Ut_DuiImToolbar::cleanup()
{
    delete m_subject;
}

void Ut_DuiImToolbar::cleanupTestCase()
{
    delete style;
    style = 0;
    LayoutsManager::destroyInstance();
    delete app;
    app = 0;
}

void Ut_DuiImToolbar::testCopyPasteButton()
{
    QList<bool> copyAvailable;
    QList<bool> pasteAvailable;
    QList<bool> visible;
    QList<QString> label;
    QList<int> expectedSignal;
    QList<CopyPasteState> signalParameter;

    DuiButton *button = m_subject->copyPaste;
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

void Ut_DuiImToolbar::testShowToolbarWidget()
{
    QSignalSpy spy(m_subject, SIGNAL(regionUpdated()));
    QVERIFY(spy.isValid());
    m_subject->showToolbarWidget(toolbarFileName);
    //toolbar buttons depend on the its data
    //including spacing widget and close button
    QCOMPARE(m_subject->leftBar.count(), 1);
    QCOMPARE(m_subject->rightBar.count(), 1);
    QCOMPARE(spy.count(), 1);
    spy.clear();
}

void Ut_DuiImToolbar::testShowGroup()
{
    m_subject->showToolbarWidget(toolbarFileName);
    //find button test2, which click will show group test
    DuiButton *button = m_subject->toolbarMgr->button("test1");
    QVERIFY(button != 0);
    button->click();
    QCOMPARE(m_subject->leftBar.count(), 1);
    QCOMPARE(m_subject->rightBar.count(), 2);
}

void Ut_DuiImToolbar::testHideGroup()
{
    m_subject->showToolbarWidget(toolbarFileName);
    //find button test3, which click will hide group test
    DuiButton *button = m_subject->toolbarMgr->button("test2");
    QVERIFY(button != 0);
    button->click();
    QCOMPARE(m_subject->leftBar.count(), 0);
    QCOMPARE(m_subject->rightBar.count(), 1);
}

void Ut_DuiImToolbar::testSendString()
{
    m_subject->showToolbarWidget(toolbarFileName);
    QSignalSpy spy(m_subject, SIGNAL(sendStringRequest(const QString &)));
    QVERIFY(spy.isValid());
    //find button test2, which click will send string
    DuiButton *button = m_subject->toolbarMgr->button("test2");
    QVERIFY(button != 0);
    button->click();
    QVERIFY(spy.count() == 1);
    spy.clear();
}

void Ut_DuiImToolbar::testKeySequenceString()
{
    m_subject->showToolbarWidget(toolbarFileName);
    //find button test3, which click will send key sequence (QKeyEvent)
    //because QKeyEvent is not supported by MetaType, use its own slot to test it.
    keyEvents = 0;
    connect(m_subject, SIGNAL(sendKeyEventRequest(const QKeyEvent &)),
            this, SLOT(receiveKeyEvent(const QKeyEvent &)));
    DuiButton *button = m_subject->toolbarMgr->button("test1");
    QVERIFY(button != 0);
    button->click();
    QVERIFY(keyEvents > 0);
    keyEvents = 0;
}

void Ut_DuiImToolbar::testHideToolbarWidget()
{
    m_subject->hideToolbarWidget();
}

void Ut_DuiImToolbar::receiveKeyEvent(const QKeyEvent &)
{
    keyEvents ++;
}

void Ut_DuiImToolbar::testCopy()
{
    QSignalSpy spy(m_subject, SIGNAL(copyPasteRequest(CopyPasteState)));
    QVERIFY(spy.isValid());

    m_subject->showToolbarWidget(toolbarFileName);
    //find button test2, which click will copy
    DuiButton *button = m_subject->toolbarMgr->button("test1");
    QVERIFY(button != 0);
    button->click();
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.first().count() == 1);
    CopyPasteState result = spy.first().first().value<CopyPasteState>();
    QCOMPARE(result, InputMethodCopy);
    spy.clear();
}

void Ut_DuiImToolbar::testPaste()
{
    QSignalSpy spy(m_subject, SIGNAL(copyPasteRequest(CopyPasteState)));
    QVERIFY(spy.isValid());

    m_subject->showToolbarWidget(toolbarFileName);
    //find button test3, which click will paste
    DuiButton *button = m_subject->toolbarMgr->button("test2");
    QVERIFY(button != 0);
    button->click();
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.first().count() == 1);
    CopyPasteState result = spy.first().first().value<CopyPasteState>();
    QCOMPARE(result, InputMethodPaste);
    spy.clear();
}


void Ut_DuiImToolbar::testRegion()
{
    QSignalSpy regionSignals(m_subject, SIGNAL(regionUpdated()));

    m_subject->showToolbarWidget(toolbarFileName);
    QCOMPARE(m_subject->rightBar.count(), 1);
    QCOMPARE(regionSignals.count(), 1);

    // Get region when there are two buttons on the right.
    QRegion regionTwoButtons = m_subject->region();

    // When the region is substracted from rightRect there should be nothing left.
    QRect rightRect = m_subject->rightBar.geometry().toRect();
    QVERIFY((QRegion(rightRect) - regionTwoButtons).isEmpty());

    // We need to add a new button, let's use groups.
    // Clicking test1 will add one button to the right.
    DuiButton *button = m_subject->toolbarMgr->button("test1");
    QVERIFY(button != 0);
    button->click();
    QCOMPARE(m_subject->rightBar.count(), 2);

    // Button added, check that regionUpdate() was emitted.
    QCOMPARE(regionSignals.count(), 2);

    // Get region when there are three buttons on the right.
    QRegion regionThreeButtons = m_subject->region();

    // Old region (two buttons) is not enough to cover rightRect
    // but the new region is.
    rightRect = m_subject->rightBar.geometry().toRect();
    QVERIFY(!(QRegion(rightRect) - regionTwoButtons).isEmpty());
    QVERIFY((QRegion(rightRect) - regionThreeButtons).isEmpty());

    m_subject->hideToolbarWidget();

    QCOMPARE(regionSignals.count(), 3);

    m_subject->hide();

    QCOMPARE(regionSignals.count(), 4);
    QVERIFY(m_subject->region().isEmpty());
}

void Ut_DuiImToolbar::testShowHideIndicatorButton()
{
    int count = 0;
    QGraphicsLayoutItem *item = m_subject->indicator;

    QVERIFY(indexOf(m_subject->rightBar.layout(), item) == -1);
    count = m_subject->rightBar.layout()->count();

    qDebug() << m_subject->rightBar.layout()->count();
    m_subject->showIndicatorButton();
    qDebug() << m_subject->rightBar.layout()->count();
    QCOMPARE((count + 1),  m_subject->rightBar.layout()->count());
    QVERIFY(indexOf(m_subject->rightBar.layout(), item) >= 0);

    m_subject->hideIndicatorButton();
    QCOMPARE(count,  m_subject->rightBar.layout()->count());
    QVERIFY(indexOf(m_subject->rightBar.layout(), item) == -1);
}

void Ut_DuiImToolbar::testIndicatorButton()
{
    DuiButton *button = m_subject->indicator;
    QSignalSpy spy(m_subject, SIGNAL(indicatorClicked()));
    QVERIFY(button != 0);
    QVERIFY(!button->isVisible());
    QVERIFY(spy.isValid());
    m_subject->setVisible(true);
    QVERIFY(m_subject->isVisible() == true);
    m_subject->showIndicatorButton();
    QVERIFY(button->isVisible());
    m_subject->indicator->click();
    QVERIFY(spy.count() == 1);
    spy.clear();
    m_subject->hideIndicatorButton();
}


void Ut_DuiImToolbar::testSetIndicatorButtonState()
{
    //create modifierLockOnInfoBanner manualy before changing indicator state
    //to avoid show/hide infobanner, which will cause crash
    m_subject->modifierLockOnInfoBanner = new DuiInfoBanner(DuiInfoBanner::Information);
    m_subject->showIndicatorButton();
    DuiGConfItem systemDisplayLanguage(SystemDisplayLanguage);

    //latin
    systemDisplayLanguage.set(QVariant("en_gb"));
    // "abc"
    QCOMPARE(m_subject->indicator->text(), LatinShiftOffIndicatorLabel);
    // -> "Abc"
    m_subject->setIndicatorButtonState(Qt::ShiftModifier, ModifierLatchedState);
    QCOMPARE(m_subject->indicator->text(), LatinShiftOnIndicatorLabel);
    // -> "ABC"
    m_subject->setIndicatorButtonState(Qt::ShiftModifier, ModifierLockedState);
    QCOMPARE(m_subject->indicator->text(), LatinShiftLockedIndicatorLabel);
    // -> "123"
    hideLockOnInfoBannerCalls = 0;
    m_subject->setIndicatorButtonState(FnLevelModifier, ModifierLatchedState);
    QCOMPARE(m_subject->indicator->text(), FNOnIndicatorLabel);
    QCOMPARE(hideLockOnInfoBannerCalls, 1);
    // -> "123_"
    m_subject->setIndicatorButtonState(FnLevelModifier, ModifierLockedState);
    QCOMPARE(m_subject->indicator->text(), FNLockedIndicatorLabel);

    //cyrillic
    systemDisplayLanguage.set(QVariant("ru"));
    m_subject->hideIndicatorButton();
    m_subject->showIndicatorButton();
    // cyrillic shiftoff
    QCOMPARE(m_subject->indicator->text(), CyrillicShiftOffIndicatorLabel);
    // cyrillic shifton
    m_subject->setIndicatorButtonState(Qt::ShiftModifier, ModifierLatchedState);
    QCOMPARE(m_subject->indicator->text(), CyrillicShiftOnIndicatorLabel);
    // cyrillic shiftlock
    m_subject->setIndicatorButtonState(Qt::ShiftModifier, ModifierLockedState);
    QCOMPARE(m_subject->indicator->text(), CyrillicShiftLockedIndicatorLabel);

    //arabic
    systemDisplayLanguage.set(QVariant("ar"));
    m_subject->hideIndicatorButton();
    m_subject->showIndicatorButton();
    // arabic shiftoff
    QCOMPARE(m_subject->indicator->text(), ArabicIndicatorLabel);
    // arabic shifton
    m_subject->setIndicatorButtonState(Qt::ShiftModifier, ModifierLatchedState);
    QCOMPARE(m_subject->indicator->text(), ArabicIndicatorLabel);
    // arabic shiftlock
    m_subject->setIndicatorButtonState(Qt::ShiftModifier, ModifierLockedState);
    QCOMPARE(m_subject->indicator->text(), ArabicIndicatorLabel);

    systemDisplayLanguage.set(QVariant("en"));
    delete m_subject->modifierLockOnInfoBanner;
    m_subject->modifierLockOnInfoBanner = 0;
    m_subject->hideIndicatorButton();
}

QTEST_APPLESS_MAIN(Ut_DuiImToolbar);

