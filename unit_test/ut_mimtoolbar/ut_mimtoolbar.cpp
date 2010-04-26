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
#include "toolbarmanager.h"
#include "toolbardata.h"
#include "mtoolbarbutton.h"
#include "mapplication.h"
#include "mvirtualkeyboard.h"
#include "layoutsmanager.h"
#include "mvirtualkeyboardstyle.h"
#include <mplainwindow.h>
#include <MTheme>
#include <MSceneWindow>
#include <MButton>
#include <MLabel>
#include <MInfoBanner>
#include "mgconfitem_stub.h"

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
    QString ToolbarFileName("/usr/share/meegotouch/imtoolbars/testtoolbar.xml");
    qlonglong ToolbarId(qrand());
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
    const QString SystemDisplayLanguage("/meegotouch/i18n/language");

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

    ToolbarManager::createInstance();
    //fill up toolbar with some data
    ToolbarData *toolbar = new ToolbarData;
    toolbar->toolbarFileName = ToolbarFileName;
    ToolbarWidget *b1 = new ToolbarWidget(ToolbarWidget::Button);
    b1->widgetName = "testbutton1";
    b1->group = "group1";
    b1->priority = 0;
    b1->orientation = M::Landscape;
    b1->showOn = ToolbarWidget::Always;
    b1->hideOn = ToolbarWidget::Undefined;
    b1->alignment = Qt::AlignLeft;
    b1->text = "testbutton1";
    b1->textId = "";
    b1->toggle = true;
    b1->pressed = false;
    ToolbarWidget::Action *action11 = new ToolbarWidget::Action(ToolbarWidget::SendKeySequence);
    action11->keys = "Ctrl+I";
    b1->actions.append(action11);
    ToolbarWidget::Action *action12 = new ToolbarWidget::Action(ToolbarWidget::Copy);
    b1->actions.append(action12);
    ToolbarWidget::Action *action13 = new ToolbarWidget::Action(ToolbarWidget::ShowGroup);
    action13->group = "group3";
    b1->actions.append(action13);
    toolbar->widgets.append(b1);

    ToolbarWidget *l1 = new ToolbarWidget(ToolbarWidget::Label);
    l1->widgetName = "testlabel1";
    l1->group = "group4";
    l1->priority = 1;
    l1->orientation = M::Landscape;
    l1->showOn = ToolbarWidget::Always;
    l1->hideOn = ToolbarWidget::WhenSelectingText;
    l1->alignment = Qt::AlignLeft;
    l1->text = "testlabel1";
    l1->textId = "";
    toolbar->widgets.append(l1);

    ToolbarWidget *b2 = new ToolbarWidget(ToolbarWidget::Button);
    b2->widgetName = "testbutton2";
    b2->group = "group2";
    b2->priority = 0;
    b2->orientation = M::Landscape;
    b2->showOn = ToolbarWidget::Always;
    b2->hideOn = ToolbarWidget::Undefined;
    b2->alignment = Qt::AlignRight;
    b2->text = "testbutton2";
    b2->textId = "";
    ToolbarWidget::Action *action21 = new ToolbarWidget::Action(ToolbarWidget::SendString);
    action21->text = "test string";
    b2->actions.append(action21);
    ToolbarWidget::Action *action22 = new ToolbarWidget::Action(ToolbarWidget::Paste);
    b2->actions.append(action22);
    ToolbarWidget::Action *action23 = new ToolbarWidget::Action(ToolbarWidget::HideGroup);
    action23->group = "group1";
    b2->actions.append(action23);
    toolbar->widgets.append(b2);

    ToolbarWidget *b3 = new ToolbarWidget(ToolbarWidget::Button);
    b3->widgetName = "testbutton3";
    b3->group = "group3";
    b3->priority = 1;
    b3->orientation = M::Landscape;
    b3->showOn = ToolbarWidget::WhenSelectingText;
    b3->hideOn = ToolbarWidget::Undefined;
    b3->alignment = Qt::AlignRight;
    b3->text = "testbutton3";
    b3->textId = "";
    toolbar->widgets.append(b3);

    ToolbarManager::instance().toolbars.insert(ToolbarId, ToolbarFileName);
    ToolbarManager::instance().cachedToolbars.insert(ToolbarId, toolbar);
    ToolbarManager::instance().cachedToolbarIds.prepend(ToolbarId);
}


void Ut_MImToolbar::init()
{
    m_subject = new MImToolbar(*style);
}


void Ut_MImToolbar::cleanup()
{
    delete m_subject;
}

void Ut_MImToolbar::cleanupTestCase()
{
    ToolbarManager::destroyInstance();
    LayoutsManager::destroyInstance();
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
    m_subject->showToolbarWidget(ToolbarId);
    //toolbar buttons depend on the its data
    //including spacing widget and close button
    QCOMPARE(m_subject->leftBar.count(), 2);
    QCOMPARE(m_subject->rightBar.count(), 1);
    QCOMPARE(spy.count(), 1);
    spy.clear();
}

void Ut_MImToolbar::testShowGroup()
{
    m_subject->showToolbarWidget(ToolbarId);
    //find button testbutton2, which click will show group test
    MButton *button = qobject_cast<MButton *>(m_subject->toolbarMgr.widget("testbutton1"));
    QVERIFY(button != 0);
    button->click();
    QCOMPARE(m_subject->leftBar.count(), 2);
    QCOMPARE(m_subject->rightBar.count(), 2);
}

void Ut_MImToolbar::testHideGroup()
{
    m_subject->showToolbarWidget(ToolbarId);
    //find button testbutton2, which click will hide group test
    MButton *button = qobject_cast<MButton *>(m_subject->toolbarMgr.widget("testbutton2"));
    QVERIFY(button != 0);
    button->click();
    QCOMPARE(m_subject->leftBar.count(), 1);
    QCOMPARE(m_subject->rightBar.count(), 1);
}

void Ut_MImToolbar::testSendString()
{
    m_subject->showToolbarWidget(ToolbarId);
    QSignalSpy spy(m_subject, SIGNAL(sendStringRequest(const QString &)));
    QVERIFY(spy.isValid());
    //find button testbutton2, which click will send string
    MButton *button = qobject_cast<MButton *>(m_subject->toolbarMgr.widget("testbutton2"));
    QVERIFY(button != 0);
    button->click();
    QVERIFY(spy.count() == 1);
    spy.clear();
}

void Ut_MImToolbar::testKeySequenceString()
{
    m_subject->showToolbarWidget(ToolbarId);
    //find button testbutton1, which click will send key sequence (QKeyEvent)
    //because QKeyEvent is not supported by MetaType, use its own slot to test it.
    keyEvents = 0;
    connect(m_subject, SIGNAL(sendKeyEventRequest(const QKeyEvent &)),
            this, SLOT(receiveKeyEvent(const QKeyEvent &)));
    MButton *button = qobject_cast<MButton *>(m_subject->toolbarMgr.widget("testbutton1"));
    QVERIFY(button != 0);
    button->click();
    QVERIFY(keyEvents > 0);
    keyEvents = 0;
}

void Ut_MImToolbar::testHideToolbarWidget()
{
    m_subject->showToolbarWidget(ToolbarId);
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

    m_subject->showToolbarWidget(ToolbarId);
    //find button testbutton2, which click will copy
    MButton *button = qobject_cast<MButton *>(m_subject->toolbarMgr.widget("testbutton1"));
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

    m_subject->showToolbarWidget(ToolbarId);
    //find button testbutton2, which click will paste
    MButton *button = qobject_cast<MButton *>(m_subject->toolbarMgr.widget("testbutton2"));
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

    m_subject->showToolbarWidget(ToolbarId);
    QCOMPARE(m_subject->rightBar.count(), 1);
    QCOMPARE(regionSignals.count(), 1);

    // Get region when there are two buttons on the right.
    QRegion regionTwoButtons = m_subject->region();

    // When the region is substracted from rightRect there should be nothing left.
    QRect rightRect = m_subject->rightBar.geometry().toRect();
    QVERIFY((QRegion(rightRect) - regionTwoButtons).isEmpty());

    // We need to add a new button, let's use groups.
    // Clicking testbutton1 will add one button to the right.
    MButton *button = qobject_cast<MButton *>(m_subject->toolbarMgr.widget("testbutton1"));
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

void Ut_MImToolbar::testShowHideIndicatorButton()
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

void Ut_MImToolbar::testIndicatorButton()
{
    MButton *button = m_subject->indicator;
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


void Ut_MImToolbar::testSetIndicatorButtonState()
{
    m_subject->showIndicatorButton();
    MGConfItem systemDisplayLanguage(SystemDisplayLanguage);

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
    m_subject->setIndicatorButtonState(FnLevelModifier, ModifierLatchedState);
    QCOMPARE(m_subject->indicator->text(), FNOnIndicatorLabel);
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
    m_subject->hideIndicatorButton();
}

void Ut_MImToolbar::testSetToolbarItemAttribute()
{
    m_subject->showToolbarWidget(ToolbarId);
    QString text("dummy_label");

    MToolbarButton *button = qobject_cast<MToolbarButton *>(m_subject->toolbarMgr.widget("testbutton1"));
    QVERIFY(button != 0);
    QCOMPARE(button->isCheckable(), true);
    QCOMPARE(button->isChecked(), false);

    MLabel *label = qobject_cast<MLabel *>(m_subject->toolbarMgr.widget("testlabel1"));
    QVERIFY(label != 0);

    ToolbarManager::instance().setToolbarItemAttribute(ToolbarId, "testlabel1", "text", text);
    QCOMPARE(label->text(), text);

    ToolbarManager::instance().setToolbarItemAttribute(ToolbarId, "testbutton1", "text", text);
    QCOMPARE(button->text(), text);

    ToolbarManager::instance().setToolbarItemAttribute(ToolbarId, "testbutton1", "pressed", QVariant(true));
    QCOMPARE(button->isChecked(), true);

    ToolbarManager::instance().setToolbarItemAttribute(ToolbarId, "testbutton1", "pressed", QVariant(false));
    QCOMPARE(button->isChecked(), false);

    QString icon("dummy_icon");
    ToolbarManager::instance().setToolbarItemAttribute(ToolbarId, "testbutton1", "icon", icon);
    QCOMPARE(button->iconFile, icon);
    ToolbarManager::instance().setToolbarItemAttribute(ToolbarId, "testbutton1", "icon", QString(""));
    QCOMPARE(button->iconFile, QString(""));
}

QTEST_APPLESS_MAIN(Ut_MImToolbar);

