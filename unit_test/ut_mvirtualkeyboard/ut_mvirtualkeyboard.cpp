/* * This file is part of m-keyboard *
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



#include "mapplication.h"
#include "mgconfitem_stub.h"
#include "duireactionmaptester.h"
#include "mvirtualkeyboard.h"
#include "horizontalswitcher.h"
#include "layoutsmanager.h"
#include "notification.h"
#include "ut_mvirtualkeyboard.h"
#include "vkbdatakey.h"
#include "mimtoolbar.h"
#include "mplainwindow.h"
#include <MScene>
#include <MSceneManager>
#include <MSceneWindow>
#include <QDebug>
#include <QFile>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QSignalSpy>
#include <QTimer>
#include <QGraphicsLayout>
#include <mtexteditmodel.h>
#include <mtheme.h>
#include <layoutmenu.h>

namespace
{
    const int SceneRotationTime = 1400; // in ms
}

Q_DECLARE_METATYPE(KeyBinding::KeyAction);

// STUBS

MApplication::~MApplication()
{
}

LayoutMenu::~LayoutMenu()
{
//this stub allows to avoid crash in libmeegotouch
}

void Notification::displayText(const QString &message)
{
    Q_UNUSED(message);
    qDebug() << __PRETTY_FUNCTION__ << __FILE__ << __LINE__;
}


void Ut_MVirtualKeyboard::initTestCase()
{
    // Avoid waiting if im server is not responding
    MApplication::setLoadMInputContext(false);

    static char *argv[2] = {(char *) "ut_mvirtualkeyboard", (char *) "-software"};
    static int argc = 2;
    app = new MApplication(argc, argv);

    QString InputMethodSetting("/meegotouch/inputmethods/languages");
    MGConfItem item1(InputMethodSetting);

    QStringList langlist;
    langlist << "en_GB" << "fi" << "ar_SA";
    item1.set(QVariant(langlist));

    QString DefaultLanguageSetting("/meegotouch/inputmethods/languages/default");
    MGConfItem item2(DefaultLanguageSetting);
    QString defaultlanguage = "en_GB";
    item2.set(QVariant(defaultlanguage));

    QString pixmapDirectory("../../m-keyboard/theme");
    if (!QFile::exists(pixmapDirectory)) {
        pixmapDirectory = "/usr/share/meegotouch/virtual-keyboard/images";
        QVERIFY(QFile::exists(pixmapDirectory));
    }
    MTheme::instance()->addPixmapDirectory(pixmapDirectory);

    QString cssFile("../../m-keyboard/theme/864x480.css");
    if (!QFile::exists(cssFile)) {
        cssFile = "/usr/share/meegotouch/virtual-keyboard/css/864x480.css";
        QVERIFY(QFile::exists(cssFile));
    }
    MTheme::instance()->loadCSS(cssFile);

    // MVirtualkeyboard uses MPlainWindow internally so we need to instantiate it.
    new MPlainWindow; // creates a static instance

    vkbParent = new MSceneWindow;
    vkbParent->setManagedManually(true); // we want the scene window to remain in origin
    // Adds scene window to scene.
    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(vkbParent);

    LayoutsManager::createInstance();
}

void Ut_MVirtualKeyboard::cleanupTestCase()
{
    LayoutsManager::destroyInstance();
    delete vkbParent;
    delete MPlainWindow::instance();
    delete app;
    app = 0;
}

void Ut_MVirtualKeyboard::init()
{
    m_vkb = new MVirtualKeyboard(LayoutsManager::instance(), vkbParent);

    connect(m_vkb, SIGNAL(regionUpdated(const QRegion &)), m_vkb, SLOT(redrawReactionMaps()));

    if (MPlainWindow::instance()->orientationAngle() != M::Angle0)
        rotateToAngle(M::Angle0);
}

void Ut_MVirtualKeyboard::cleanup()
{
    // Stub might have been set to a local instance.
    gDuiReactionMapStub = &gDefaultDuiReactionMapStub;
    delete m_vkb;
}

void Ut_MVirtualKeyboard::clickBackspaceTest()
{
#if 0 // DISABLED: functionKeysMap not available
    FlickUpButton *button = 0;

    for (int i = 0; i < numFunctionKeys; i++) {
        const VKBDataKey *key = functionkeySection->getVKBKey(0, 0, i);
        if (key->action == VKBDataKey::ActionBackspace)
            button = static_cast<FlickUpButton *>(m_vkb->functionKeysMap.value(i));
    }
    QVERIFY(button != 0);

    QSignalSpy spy(m_vkb, SIGNAL(clicked(const QString &)));
    QSignalSpy pressed(m_vkb, SIGNAL(keyPressed(const QChar &)));
    //TODO:there are bugs in mbutton, we can not simulate the keypress and keyrelease...
    m_vkb->pressBackspace();
    button->setDown(false);
    m_vkb->autoBackspace();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst(); // take the first signal
    QVERIFY(arguments.at(0).toString() == "\b"); // verify the first argument

    QCOMPARE(pressed.count(), 1);
    arguments = pressed.takeFirst();
    QVERIFY(arguments.count() == 1);
    QVERIFY(arguments.first().toChar() == QChar('\b'));
#endif
}

void Ut_MVirtualKeyboard::clickSpaceTest()
{
#if 0
    FlickUpButton *button = 0;

    for (int i = 0; i < numFunctionKeys; i++) {
        const VKBDataKey *key = functionkeySection->getVKBKey(0, 0, i);
        if (key->action == VKBDataKey::ActionSpace)
            button = static_cast<FlickUpButton *>(m_vkb->functionKeysMap.value(i));
    }
    QVERIFY(button != 0);
    QVERIFY(m_vkb->shiftLevel == MVirtualKeyboard::ShiftOff);

    QSignalSpy spy(m_vkb, SIGNAL(clickedSymbolButton(const QString &)));
    button->click();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    QVERIFY(arguments.at(0).toString() == " "); // verify the first argument

    m_vkb->clickShift();
    QVERIFY(m_vkb->shiftLevel == MVirtualKeyboard::ShiftOn);
    spy.clear();

    button->click();

    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst(); // take the first signal

    QVERIFY(arguments.at(0).toString() == "\n"); // verify the first argument
#endif
}

void Ut_MVirtualKeyboard::setShiftStateTest()
{
    QList<MVirtualKeyboard::ShiftLevel> levels;
    levels << MVirtualKeyboard::ShiftOn << MVirtualKeyboard::ShiftOff
           << MVirtualKeyboard::ShiftLock;
    foreach(MVirtualKeyboard::ShiftLevel level, levels) {
        m_vkb->setShiftState(level);
        QCOMPARE(m_vkb->shiftStatus(), level);
    }
}

void Ut_MVirtualKeyboard::clickHyphenTest()
{
#if 0
    FlickUpButton *button = 0;
    for (int i = 0; i < numFunctionKeys; i++) {
        const VKBDataKey *key = functionkeySection->getVKBKey(0, 0, i);

        if (key->label == "-")
            button = static_cast<FlickUpButton *>(m_vkb->functionKeysMap.value(i));
    }
    QVERIFY(button != 0);

    QSignalSpy spy(m_vkb, SIGNAL(clickedSymbolButton(const QString &)));
    button->click();
    QCOMPARE(spy.count(), 1);

    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    QVERIFY(arguments.at(0).toString() == "-"); // verify the first argument
#endif
}


void Ut_MVirtualKeyboard::clickPunctQuesTest()
{
#if 0
    FlickUpButton *button = 0;
    for (int i = 0; i < numFunctionKeys; i++) {
        const VKBDataKey *key = functionkeySection->getVKBKey(0, 0, i);

        if (key->label == "?")
            button = static_cast<FlickUpButton *>(m_vkb->functionKeysMap.value(i));
    }
    QVERIFY(button != 0);

    QSignalSpy spy(m_vkb, SIGNAL(clickedSymbolButton(const QString &)));
    button->click();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    QVERIFY(arguments.at(0).toString() == "?"); // verify the first argument
#endif
}

void Ut_MVirtualKeyboard::clickPunctDotTest()
{
#if 0
    FlickUpButton *button = 0;

    for (int i = 0; i < numFunctionKeys; i++) {
        const VKBDataKey *key = functionkeySection->getVKBKey(0, 0, i);

        if (key->label == ".")
            button = static_cast<FlickUpButton *>(m_vkb->functionKeysMap.value(i));
    }
    QVERIFY(button != 0);

    QSignalSpy spy(m_vkb, SIGNAL(clickedSymbolButton(const QString &)));
    button->click();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    QVERIFY(arguments.at(0).toString() == "."); // verify the first argument
#endif
}


void Ut_MVirtualKeyboard::fadeTest()
{
    m_vkb->fade(100);
    QCOMPARE(m_vkb->opacity(), 1.0);
}

void Ut_MVirtualKeyboard::regionSuppressionTest()
{
    qRegisterMetaType<QRegion>("QRegion");
    QSignalSpy spy(m_vkb, SIGNAL(regionUpdated(QRegion)));
    m_vkb->sendVKBRegion();
    QCOMPARE(spy.count(), 1);
    spy.clear();

    // Need to get region signal in a delayed fashion after enabling updates
    m_vkb->suppressRegionUpdate(true);
    m_vkb->sendVKBRegion();
    m_vkb->suppressRegionUpdate(false);
    QCOMPARE(spy.count(), 1);
    spy.clear();

    // Just one signal even in the case of multiple send requests
    m_vkb->suppressRegionUpdate(true);
    m_vkb->sendVKBRegion();
    m_vkb->sendVKBRegion();
    m_vkb->suppressRegionUpdate(false);
    QCOMPARE(spy.count(), 1);
    spy.clear();

    // No signal when no send requests
    m_vkb->suppressRegionUpdate(true);
    m_vkb->suppressRegionUpdate(false);
    QCOMPARE(spy.count(), 0);
}

void Ut_MVirtualKeyboard::showKeyboardTest()
{
    QSignalSpy spy(m_vkb, SIGNAL(regionUpdated(QRegion)));
    m_vkb->showKeyboard();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);
    QCOMPARE(spy.count(), 2);
    QVERIFY(!spy.takeFirst().at(0).value<QRegion>().isEmpty());

    //verify that VKB will pe redrawn only when needed
    spy.clear();
    m_vkb->showKeyboard();
    QCOMPARE(spy.count(), 0);
}

void Ut_MVirtualKeyboard::hideKeyboardTest()
{
    // Note that signal is emitted only after hide animation is finished, so we
    // need to wait after hide calls.

    m_vkb->showKeyboard();  // Without this hiding won't send the signal
    QSignalSpy spy(m_vkb, SIGNAL(regionUpdated(QRegion)));
    m_vkb->hideKeyboard();
    QCOMPARE(spy.count(), 0);

    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).value<QRegion>().isEmpty());
    QVERIFY(m_vkb->region(true).isEmpty());

    // verify that no signal is emitted if vkb is already hidden
    spy.clear();
    m_vkb->hideKeyboard();
    QCOMPARE(spy.count(), 0);
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);
    QCOMPARE(spy.count(), 0);
}

void Ut_MVirtualKeyboard::testStateReset()
{
    // Open keyboard.
    m_vkb->showKeyboard();
    QCOMPARE(m_vkb->activity, MVirtualKeyboard::Active);

    // Set states that should be changed next time opening vkb.
    m_vkb->setShiftState(MVirtualKeyboard::ShiftOn); // Shift on

    // Test after reopening the keyboard, rather than after closing.
    m_vkb->hideKeyboard();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);
    m_vkb->showKeyboard();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);

    QCOMPARE(m_vkb->shiftStatus(), MVirtualKeyboard::ShiftOff); // Shift should be off
}

void Ut_MVirtualKeyboard::switchLevelTest()
{
    m_vkb->shiftLevel = MVirtualKeyboard::ShiftLock;
    m_vkb->switchLevel();
    QCOMPARE(m_vkb->currentLevel, 1);
}

void Ut_MVirtualKeyboard::flickRightHandlerTest()
{
    QStringList langList = MGConfItem("/meegotouch/inputmethods/languages").value().toStringList();
    langList.sort();

    // Vkb has just been created and is running an animation.
    QTest::qWait(550);

    // flick right, switch direction left
    QSignalSpy spy(m_vkb, SIGNAL(languageChanged(const QString &)));
    int index = m_vkb->mainKeyboardSwitcher->current();
    m_vkb->flickRightHandler();
    QVERIFY(m_vkb->mainKeyboardSwitcher->current() != index);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).count(), 1);
    //depends on the languages set through MGConfItem
    QCOMPARE(spy.at(0).at(0).toString().toLower(), langList.at(0).toLower());

    QTest::qWait(550);

    index = m_vkb->mainKeyboardSwitcher->current();
    m_vkb->flickRightHandler();
    QVERIFY(m_vkb->mainKeyboardSwitcher->current() != index);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).count(), 1);
    //depends on the languages set through MGConfItem
    QCOMPARE(spy.at(1).at(0).toString().toLower(), langList.at(2).toLower());

    QTest::qWait(550);

    index = m_vkb->mainKeyboardSwitcher->current();
    m_vkb->flickRightHandler();
    QVERIFY(m_vkb->mainKeyboardSwitcher->current() != index);
    QCOMPARE(spy.count(), 3);
    QCOMPARE(spy.at(2).count(), 1);
    //depends on the languages set through MGConfItem
    QCOMPARE(spy.at(2).at(0).toString().toLower(), langList.at(1).toLower());
}

void Ut_MVirtualKeyboard::flickLeftHandlerTest()
{
    QStringList langList = MGConfItem("/meegotouch/inputmethods/languages").value().toStringList();
    langList.sort();

    // Vkb has just been created and is running an animation.
    QTest::qWait(550);

    QSignalSpy spy(m_vkb, SIGNAL(languageChanged(const QString &)));
    int index = m_vkb->mainKeyboardSwitcher->current();
    qDebug() << "index1: " << index << " lang:" << m_vkb->currentLanguage;
    m_vkb->flickLeftHandler();
    qDebug() << "index2: " << m_vkb->mainKeyboardSwitcher->current() << " lang:" << m_vkb->currentLanguage;
    QVERIFY(m_vkb->mainKeyboardSwitcher->current() != index);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).count(), 1);
    //depends on the languages set through MGConfItem
    QCOMPARE(spy.at(0).at(0).toString().toLower(), langList.at(2).toLower());

    QTest::qWait(550);

    index = m_vkb->mainKeyboardSwitcher->current();
    m_vkb->flickLeftHandler();
    QVERIFY(m_vkb->mainKeyboardSwitcher->current() != index);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).count(), 1);
    //depends on the languages set through MGConfItem
    QCOMPARE(spy.at(1).at(0).toString().toLower(), langList.at(0).toLower());

    QTest::qWait(550);

    index = m_vkb->mainKeyboardSwitcher->current();
    m_vkb->flickLeftHandler();
    QVERIFY(m_vkb->mainKeyboardSwitcher->current() != index);
    QCOMPARE(spy.count(), 3);
    QCOMPARE(spy.at(2).count(), 1);
    //depends on the languages set through MGConfItem
    QCOMPARE(spy.at(2).at(0).toString().toLower(), langList.at(1).toLower());
}

void Ut_MVirtualKeyboard::loadSymbolViewTemporarilyTest()
{
#if 0 // method not available anymore
    QVariant val(1);
    m_vkb->loadSCVTemporarily();
    QCOMPARE(m_vkb->scv->data(0), val);
#endif
}

void Ut_MVirtualKeyboard::errorCorrectionTest()
{
#if 0
    QSignalSpy spy(m_vkb, SIGNAL(errorCorrectionToggled(bool)));

    m_vkb->disableErrorCorrection();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), false);

    m_vkb->enableErrorCorrection();
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).count(), 1);
    QCOMPARE(spy.at(1).at(0).toBool(), true);
#endif
}

void Ut_MVirtualKeyboard::setKeyboardType()
{
    m_vkb->setKeyboardType(M::NumberContentType);
    QCOMPARE(m_vkb->currentLayoutType, LayoutData::Number);

    m_vkb->setKeyboardType(M::PhoneNumberContentType);
    QCOMPARE(m_vkb->currentLayoutType, LayoutData::PhoneNumber);

    m_vkb->setKeyboardType(M::FreeTextContentType);
    QCOMPARE(m_vkb->currentLayoutType, LayoutData::General);

    m_vkb->setKeyboardType(M::EmailContentType);
    QCOMPARE(m_vkb->currentLayoutType, LayoutData::General);

    m_vkb->setKeyboardType(M::UrlContentType);
    QCOMPARE(m_vkb->currentLayoutType, LayoutData::General);

    m_vkb->setKeyboardType(M::CustomContentType);
    QCOMPARE(m_vkb->currentLayoutType, LayoutData::General);
}

void Ut_MVirtualKeyboard::longPressBackSpace()
{
#if 0
    FlickUpButton *button = 0;

    for (int i = 0; i < numFunctionKeys; i++) {
        const VKBDataKey *key = functionkeySection->getVKBKey(0, 0, i);

        if (key->action == VKBDataKey::ActionBackspace)
            button = static_cast<FlickUpButton *>(m_vkb->functionKeysMap.value(i));
    }
    QVERIFY(button != 0);

    QSignalSpy spy(m_vkb, SIGNAL(clicked(const QString &)));
    spy.clear();
    //TODO:there are bugs in mbutton, we can not simulate the keypress and keyrelease...
    button->setDown(false);
    button->setDown(true);
    m_vkb->pressBackspace();
    QCOMPARE(spy.count(), 0);

    QList<QVariant> arguments;

    spy.clear();
    //500ms, means holding, and one signal should be emit
    QTest::qWait(550);
    QVERIFY(m_vkb->backSpaceTimer->isActive() == true);
    QVERIFY(button->isDown());
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst(); // take the first signal
    QVERIFY(arguments.count() == 1);
    QVERIFY(arguments.at(0).toString() == "\b"); // verify the first argument
    spy.clear();
    //another 100ms means repeat auto delete, and one more signal should be emit
    QTest::qWait(100);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst(); // take the first signal
    QVERIFY(arguments.count() == 1);
    QVERIFY(arguments.at(0).toString() == "\b"); // verify the first argument

    spy.clear();
    QTest::qWait(300);
    QCOMPARE(spy.count(), 3);
    for (int i = 0; i < spy.count(); i++) {
        arguments = spy.takeAt(i); //take each signal
        QVERIFY(arguments.count() == 1);
        QVERIFY(arguments.at(0).toString() == "\b"); // verify the first argument
    }

    spy.clear();
    QTest::qWait(500);
    QCOMPARE(spy.count(), 5);
    for (int i = 0; i < spy.count(); i++) {
        arguments = spy.takeAt(i); //take each singal
        QVERIFY(arguments.count() == 1);
        QVERIFY(arguments.at(0).toString() == "\b"); // verify the first argument
    }

    button->setDown(false);
#endif
}

/*
 *  Bug description: symbol keys send incorrect characters
 * in uppercase mode
 */
void Ut_MVirtualKeyboard::bug_130644()
{
#if 0
    FlickUpButton *symbolButton = 0;
    QSignalSpy spy(m_vkb, SIGNAL(clickedSymbolButton(const QString &)));
    QStringList buttonLabels;

    buttonLabels << "'" << "!" << ","; //this list depends on substitute_label from EN_default.xml

    foreach(const QString & label, buttonLabels) {
        //initialization
        qDebug() << "Test button" << label;
        QCOMPARE(m_vkb->shiftStatus(), MVirtualKeyboard::ShiftOff);
        m_vkb->setShiftState(MVirtualKeyboard::ShiftOn);
        QVERIFY(m_vkb->shiftStatus() == MVirtualKeyboard::ShiftOn);

        symbolButton = 0;
        foreach(FlickUpButton * button, m_vkb->functionKeysMap.values()) {
            if (button->text() == label) {
                symbolButton = button;
                break;
            }
        }
        QVERIFY(symbolButton != 0); //we found button to click on

        spy.clear();
        //actual testing
        symbolButton->click();
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().count(), 1);
        QCOMPARE(spy.first().first().toString(), label);
    }
#endif
}

void Ut_MVirtualKeyboard::symbolKeyTestLowercase()
{
#if 0
    FlickUpButton *symbolButton = 0;
    QSignalSpy spy(m_vkb, SIGNAL(clickedSymbolButton(const QString &)));
    QStringList buttonLabels;

    buttonLabels << "-" << "?" << "."; //this list depends on substitute_label from EN_default.xml

    foreach(const QString & label, buttonLabels) {
        //initialization
        qDebug() << "Test button" << label;
        QCOMPARE(m_vkb->shiftStatus(), MVirtualKeyboard::ShiftOff);

        symbolButton = 0;
        foreach(FlickUpButton * button, m_vkb->functionKeysMap.values()) {
            if (button->text() == label) {
                symbolButton = button;
                break;
            }
        }
        QVERIFY(symbolButton != 0); //we found button to click on

        spy.clear();
        //actual testing
        symbolButton->click();
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().count(), 1);
        QCOMPARE(spy.first().first().toString(), label);
    }
#endif
}

void Ut_MVirtualKeyboard::symbolKeyTestCapsLock()
{
#if 0
    FlickUpButton *symbolButton = 0;
    ShiftButton *shift = 0;
    QSignalSpy spy(m_vkb, SIGNAL(clickedSymbolButton(const QString &)));
    QStringList buttonLabels;

    buttonLabels << "'" << "!" << ","; //this list depends on substitute_label from EN_default.xml

    QCOMPARE(m_vkb->shiftStatus(), MVirtualKeyboard::ShiftOff);

    for (int i = 0; i < numFunctionKeys; i++) {
        const VKBDataKey *key = functionkeySection->getVKBKey(0, 0, i);

        if (key->action == VKBDataKey::ActionShift)
            shift = static_cast<ShiftButton *>(m_vkb->functionKeysMap.value(i));
    }
    QVERIFY(shift != 0);
    shift->lock();

    foreach(const QString & label, buttonLabels) {
        //initialization
        qDebug() << "Test button" << label;
        QCOMPARE(m_vkb->shiftStatus(), MVirtualKeyboard::ShiftLock);

        symbolButton = 0;
        foreach(FlickUpButton * button, m_vkb->functionKeysMap.values()) {
            if (button->text() == label) {
                symbolButton = button;
                break;
            }
        }
        QVERIFY(symbolButton != 0); //we found button to click on

        spy.clear();
        //actual testing
        symbolButton->click();
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().count(), 1);
        QCOMPARE(spy.first().first().toString(), label);
    }
#endif
}

void Ut_MVirtualKeyboard::interceptPress()
{
#if 0
    QSignalSpy spy(m_vkb, SIGNAL(keyPressed(const QChar &)));
    QList<QChar> testData;
    QList<QChar> expected;

    testData << 'a' << QChar(0x21B5) << QChar(0x2192) << QChar(0x2B06)
             << QChar(0x21E7) << QChar(0x21EA);
    expected << 'a' << '\n' << ' ' << QChar() << QChar() << QChar();
    QVERIFY(testData.count() == expected.count());

    for (int n = 0; n < testData.count(); ++n) {
        qDebug() << "test step" << n << testData.at(n);
        m_vkb->interceptPress(testData.at(n));
        QVERIFY(spy.count() == 1);
        QVERIFY(spy.first().count() == 1);
        QVERIFY(spy.first().first() == expected.at(n));
        spy.clear();
    }

    m_vkb->interceptPress(QChar());
    QVERIFY(spy.count() == 0);
#endif
}

/*
 * Bug description: Sym view becomes invisible when shift button
 * is activated
 */
void Ut_MVirtualKeyboard::bug_137295()
{
#if 0 // uses scv directly
    QVERIFY(m_vkb->scv->isActive() == false);
    QVERIFY(m_vkb->shiftLevel == MVirtualKeyboard::ShiftOff);

    m_vkb->scv->tabPressed(0);
    QVERIFY(m_vkb->scv->isActive() == true);
    QVERIFY(m_vkb->scv->isVisible() == true);

    m_vkb->clickShift();
    QVERIFY(m_vkb->shiftLevel == MVirtualKeyboard::ShiftOn);
    QVERIFY(m_vkb->scv->isActive() == true);
    QVERIFY(m_vkb->scv->isVisible() == true);

    m_vkb->lockShift();
    QVERIFY(m_vkb->shiftLevel == MVirtualKeyboard::ShiftLock);
    QVERIFY(m_vkb->scv->isActive() == true);
    QVERIFY(m_vkb->scv->isVisible() == true);

    m_vkb->scv->hide();
    QVERIFY(m_vkb->shiftLevel == MVirtualKeyboard::ShiftLock);
    QVERIFY(m_vkb->scv->isActive() == false);
    QVERIFY(m_vkb->scv->isVisible() == true);
#endif
}

void Ut_MVirtualKeyboard::testSetKeyboardState()
{
    int top = 0;

    m_vkb->setKeyboardState(Hardware);
    m_vkb->showKeyboard();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);
    top = m_vkb->geometry().top();

    //show Copy button, toolbar becomes visible
    m_vkb->setCopyPasteButton(true, false);
    QVERIFY(top >= m_vkb->geometry().top());
    top = m_vkb->geometry().top();

    //show whole keyboard
    m_vkb->setKeyboardState(OnScreen);
    top -= m_vkb->layout()->itemAt(1)->geometry().height();
    QCOMPARE(top, int(m_vkb->geometry().top()));

    //show toolbar only
    m_vkb->setKeyboardState(Hardware);
    top += m_vkb->layout()->itemAt(1)->geometry().height();
    QCOMPARE(top, int(m_vkb->geometry().top()));
}

void Ut_MVirtualKeyboard::testReactionMaps()
{
    DuiReactionMapTester tester;
    gDuiReactionMapStub = &tester;

    m_vkb->setLanguage(0);

    // Show keyboard
    m_vkb->showKeyboard();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);
    QVERIFY(m_vkb->isFullyVisible());

    // Clear with transparent color
    gDuiReactionMapStub->setTransparentDrawingValue();
    gDuiReactionMapStub->setTransform(QTransform());
    gDuiReactionMapStub->fillRectangle(0, 0, gDuiReactionMapStub->width(), gDuiReactionMapStub->height());

    m_vkb->redrawReactionMaps();

    // Overall sanity test with grid points throughout the view.
    QVERIFY(tester.testReactionMapGrid(MPlainWindow::instance(), 40, 50, m_vkb->region(true)));

    // Check that all buttons are drawn with reactive color.
    QVERIFY(tester.testChildButtonReactiveAreas(MPlainWindow::instance(), m_vkb));

    // Check again with copy
    m_vkb->setCopyPasteButton(true, false);
    QVERIFY(tester.testReactionMapGrid(MPlainWindow::instance(), 40, 50, m_vkb->region(true)));
    QVERIFY(tester.testChildButtonReactiveAreas(MPlainWindow::instance(), m_vkb));

    // And without again
    m_vkb->setCopyPasteButton(false, false);
    QVERIFY(tester.testReactionMapGrid(MPlainWindow::instance(), 40, 50, m_vkb->region(true)));
    QVERIFY(tester.testChildButtonReactiveAreas(MPlainWindow::instance(), m_vkb));

    // Switch language, the chosen kb layouts are all different in terms of reaction maps they generate.
    m_vkb->setLanguage(1);
    QTest::qWait(600);
    QVERIFY(tester.testReactionMapGrid(MPlainWindow::instance(), 40, 50, m_vkb->region(true)));
    QVERIFY(tester.testChildButtonReactiveAreas(MPlainWindow::instance(), m_vkb));
}

void Ut_MVirtualKeyboard::flickUpHandlerTest_data()
{
    QTest::addColumn<KeyBinding::KeyAction>("action");
    QTest::addColumn<int>("expected");

    for (int n = 0; n < KeyBinding::NumActions; ++n) {
        QTest::newRow("") << KeyBinding::KeyAction(n)
            << (n == KeyBinding::ActionSym ? 1 : 0);
    }
}
void Ut_MVirtualKeyboard::flickUpHandlerTest()
{
    QFETCH(KeyBinding::KeyAction, action);
    QFETCH(int, expected);
    KeyBinding binding;

    binding.keyAction = action;

    QSignalSpy spy(m_vkb, SIGNAL(showSymbolViewRequested()));
    QVERIFY(spy.isValid());

    m_vkb->flickUpHandler(0);
    QCOMPARE(spy.count(), 0);

    m_vkb->flickUpHandler(&binding);
    QCOMPARE(spy.count(), expected);
}

// End of test functions!

void Ut_MVirtualKeyboard::rotateToAngle(M::OrientationAngle angle)
{
    m_vkb->prepareToOrientationChange();
    MPlainWindow::instance()->setOrientationAngle(angle);
    QTest::qWait(SceneRotationTime);// wait until MSceneManager::orientationAngle() is updated.
    m_vkb->finalizeOrientationChange();
}

QTEST_APPLESS_MAIN(Ut_MVirtualKeyboard);

