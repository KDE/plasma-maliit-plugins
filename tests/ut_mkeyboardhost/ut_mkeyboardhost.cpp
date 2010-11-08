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



#include <mimcorrectionhost.h>
#include <mvirtualkeyboard.h>
#include <mhardwarekeyboard.h>
#include <mkeyboardhost.h>
#include <mvirtualkeyboardstyle.h>
#include <symbolview.h>
#include <sharedhandlearea.h>
#include <mimenginewordsinterfacefactory.h>
#include <mtoolbardata.h>
#include <mimtoolbar.h>
#include <layoutsmanager.h>

#include "mgconfitem_stub.h"
#include "minputmethodhoststub.h"
#include "ut_mkeyboardhost.h"
#include "utils.h"
#include "dummydriver_mkh.h"

#include <MApplication>
#include <MSceneManager>
#include <MTheme>
#include "mplainwindow.h"
#include <mnamespace.h>
#include <MWidgetController>
#include <MDialog>

#include <QGraphicsLayout>
#include <QDir>

#include <X11/X.h>
#undef KeyPress
#undef KeyRelease

namespace
{
    const char * const XkbLayoutSettingName("/meegotouch/inputmethods/hwkeyboard/layout");
    const char * const XkbVariantSettingName("/meegotouch/inputmethods/hwkeyboard/variant");

    const QString InputMethodCorrectionSetting("/meegotouch/inputmethods/virtualkeyboard/correctionenabled");
    const QString InputMethodCorrectionEngine("/meegotouch/inputmethods/correctionengine");
    int gSetKeyboardStateCallCount = 0;
    MInputMethod::HandlerState gSetKeyboardStateParam = MInputMethod::OnScreen;
    const int SceneRotationTime = 1400; // in ms
    bool gAutoCapsEnabled = true;

    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";

    int gShowToolbarWidgetCalls = 0;
    int gHideToolbarWidgetCalls = 0;

    const Qt::Key FnLevelKey = Qt::Key_AltGr;
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;

    int gShowLockOnInfoBannerCallCount = 0;
    int gHideLockOnInfoBannerCallCount = 0;
    int gRequestLanguageNotificationCallCount = 0;

    const char * const TargetSettingsName("/meegotouch/target/name");
    const char * const DefaultTargetName("Default");
}

namespace QTest
{
    template <>
    char *toString(const QRegion &region)
    {
        QString string;
        QDebug debug(&string);

        debug << region;

        return qstrdup(qPrintable(string));
    }
}

Q_DECLARE_METATYPE(QSet<MInputMethod::HandlerState>)
Q_DECLARE_METATYPE(MInputMethod::HandlerState)
Q_DECLARE_METATYPE(ModifierState)
Q_DECLARE_METATYPE(Ut_MKeyboardHost::TestOpList)
Q_DECLARE_METATYPE(MInputMethod::InputModeIndicator)

// Stubbing..................................................................

void MVirtualKeyboard::setKeyboardState(MInputMethod::HandlerState state)
{
    ++gSetKeyboardStateCallCount;
    gSetKeyboardStateParam = state;
}

QString MVirtualKeyboard::layoutLanguage() const
{
    return QString("fi");
}

bool MVirtualKeyboard::autoCapsEnabled() const
{
    return gAutoCapsEnabled;
}

void MVirtualKeyboard::requestLanguageNotification()
{
    ++gRequestLanguageNotificationCallCount;
}

bool MHardwareKeyboard::autoCapsEnabled() const
{
    return gAutoCapsEnabled;
}

void MImToolbar::showToolbarWidget(QSharedPointer<const MToolbarData> )
{
    ++gShowToolbarWidgetCalls;
}

void MImToolbar::hideToolbarWidget()
{
    ++gHideToolbarWidgetCalls;
}

void MKeyboardHost::showLockOnInfoBanner(const QString &)
{
    gShowLockOnInfoBannerCallCount++;
}

void MKeyboardHost::hideLockOnInfoBanner()
{
    gHideLockOnInfoBannerCallCount++;
}

// Actual test...............................................................

void Ut_MKeyboardHost::initTestCase()
{
    static int argc = 2;
    static char *app_name[2] = { (char *)"ut_mvirtualkeyboardhost",
                                 (char *) "-local-theme" };

    disableQtPlugins();

    MGConfItem target(TargetSettingsName);
    target.set(DefaultTargetName); // this value is required by the theme daemon

    app = new MApplication(argc, app_name);
    inputMethodHost = new MInputMethodHostStub;
    window = new MPlainWindow;

    MGConfItem(MultitouchSettings).set(true);

    qRegisterMetaType<M::Orientation>("M::Orientation");
    qRegisterMetaType<TestOpList>("TestOpList");
}

void Ut_MKeyboardHost::cleanupTestCase()
{
    delete window;
    window = 0;
    delete inputMethodHost;
    inputMethodHost = 0;
    delete app;
    app = 0;
}

void Ut_MKeyboardHost::init()
{
    // Uses dummy driver
    MGConfItem engineConfig(InputMethodCorrectionEngine);
    engineConfig.set(QVariant(QString("dummyimdriver")));
    MGConfItem config(InputMethodCorrectionSetting);
    config.set(QVariant(false));

    subject = new MKeyboardHost(inputMethodHost, 0);
    inputMethodHost->clear();
    gAutoCapsEnabled = true;

    window->hide();
    if (window->orientationAngle() != M::Angle0) {
        window->setOrientationAngle(M::Angle0);
        QCOMPARE(window->orientationAngle(), M::Angle0);
        // Rotation is immediate if window is hidden.
    }
}

void Ut_MKeyboardHost::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_MKeyboardHost::testCreate()
{
    QVERIFY(subject != 0);
}

void Ut_MKeyboardHost::testOrientationAngleLocked()
{
    // Our window must not listen to device orientation changes.
    QVERIFY(MPlainWindow::instance()->isOrientationAngleLocked());
}

void Ut_MKeyboardHost::testRotatePoint()
{
    const QPoint screenPos(100, 200);
    QPoint result;
    int displayWidth = window->visibleSceneSize(M::Landscape).width();
    int displayHeight = window->visibleSceneSize(M::Landscape).height();
    bool isOk = false;

    rotateToAngle(M::Angle0);
    QCOMPARE(static_cast<int>(subject->angle), 0);
    isOk = subject->rotatePoint(screenPos, result);
    QVERIFY(isOk == true);
    QCOMPARE(result, screenPos);

    rotateToAngle(M::Angle90);
    QCOMPARE(static_cast<int>(subject->angle), 90);
    isOk = subject->rotatePoint(screenPos, result);
    QVERIFY(isOk == true);
    QCOMPARE(result, QPoint(200, displayWidth - 100));

    rotateToAngle(M::Angle180);
    QCOMPARE(static_cast<int>(subject->angle), 180);
    isOk = subject->rotatePoint(screenPos, result);
    QVERIFY(isOk == true);
    QCOMPARE(result, QPoint(displayWidth - 100, displayHeight - 200));

    rotateToAngle(M::Angle270);
    QCOMPARE(static_cast<int>(subject->angle), 270);
    isOk = subject->rotatePoint(screenPos, result);
    QVERIFY(isOk == true);
    QCOMPARE(result, QPoint(displayHeight - 200, 100));
}


void Ut_MKeyboardHost::testRotateRect()
{
    QRect rect;
    QRect result(1, 2, 3, 4);
    int displayWidth = MPlainWindow::instance()->visibleSceneSize().width();
    int displayHeight = MPlainWindow::instance()->visibleSceneSize().height();

    QList<QRect> rects;
    QList<M::OrientationAngle> angles;
    QList<QRect> expected;

    // invalid rectangles
    rects.append(QRect(100, 200, -20, 40));
    angles.append(M::Angle0);
    expected.append(QRect());

    rects.append(QRect(0, 0, -1, -1));
    angles.append(M::Angle90);
    expected.append(QRect());

    rects.append(QRect(1, 1, 1, -1));
    angles.append(M::Angle180);
    expected.append(QRect());

    // invalid angle
    rects.append(QRect(1, 1, 1, 1));
    angles.append((M::OrientationAngle)(-1));
    expected.append(QRect());

    // valid rectangles
    rect = QRect(100, 200, 20, 40);
    rects.append(rect);
    angles.append(M::Angle0);
    expected.append(rect);

    rect = QRect(100, 200, 20, 40);
    rects.append(rect);
    angles.append(M::Angle90);
    expected.append(QRect(rect.y(), displayWidth - rect.x() - rect.width(), rect.height(), rect.width()));

    rect = QRect(-4, 4, 20, 40);
    rects.append(rect);
    angles.append(M::Angle180);
    expected.append(QRect(
                        displayWidth - rect.x() - rect.width(),
                        displayHeight - rect.y() - rect.height(),
                        rect.width(), rect.height()));

    rect = QRect(10, 200, 20, 40);
    rects.append(rect);
    angles.append(M::Angle270);
    expected.append(QRect(
                        displayHeight - rect.y() - rect.height(), rect.x(),
                        rect.height(), rect.width()));

    for (int i = 0; i < rects.length(); ++i) {
        rect = rects.at(i);
        M::OrientationAngle angle = angles.at(i);
        rotateToAngle(angle);

        bool validAngle = (
                              angle == M::Angle0   ||
                              angle == M::Angle90  ||
                              angle == M::Angle180 ||
                              angle == M::Angle270);
        bool rotated = subject->rotateRect(rect, result);
        QCOMPARE(rotated, rect.isValid() && validAngle);
        QCOMPARE(result, expected.at(i));
    }
}


void Ut_MKeyboardHost::testHandleClick()
{
    MGConfItem config(InputMethodCorrectionSetting);
    config.set(QVariant(true));

    subject->update();
    subject->handleKeyClick(KeyEvent("a"));
    qDebug() << "correctionEnabled:" << subject->correctionEnabled;
    qDebug() << "subject->preedit:" << subject->preedit;
    QCOMPARE(subject->preedit, QString("a"));
    QCOMPARE(inputMethodHost->preedit, QString("a"));

    subject->handleKeyClick(KeyEvent(" ", QEvent::KeyRelease, Qt::Key_Space));
    QVERIFY(subject->preedit.isEmpty());
    QCOMPARE(inputMethodHost->commit, QString("a "));
    inputMethodHost->clear();

    subject->handleKeyClick(KeyEvent("a"));
    subject->handleKeyClick(KeyEvent("\r", QEvent::KeyRelease, Qt::Key_Return));
    QVERIFY(subject->preedit.isEmpty());
    QCOMPARE(inputMethodHost->commit, QString("a\r"));
    inputMethodHost->clear();

    subject->handleKeyClick(KeyEvent("a"));
    subject->handleKeyPress(KeyEvent("\b", QEvent::KeyPress, Qt::Key_Backspace));
    subject->handleKeyRelease(KeyEvent("\b", QEvent::KeyRelease, Qt::Key_Backspace));
    QVERIFY(subject->preedit.isEmpty());
    inputMethodHost->clear();

    // turn off error correction
    config.set(QVariant(false));
    QTest::qWait(100);
    QCOMPARE(subject->imCorrectionEngine->correctionEnabled(), false);
    QCOMPARE(subject->correctionEnabled, false);

    subject->handleKeyClick(KeyEvent("m"));
    subject->handleKeyClick(KeyEvent("a"));
    QCOMPARE(inputMethodHost->commit, QString("ma"));
    QVERIFY(subject->preedit.isEmpty());

    // turn on error correction
    config.set(QVariant(true));
    QTest::qWait(100);
    QCOMPARE(subject->imCorrectionEngine->correctionEnabled(), true);
    QCOMPARE(subject->correctionEnabled, true);

    subject->handleKeyPress(KeyEvent("\b", QEvent::KeyPress, Qt::Key_Backspace));
    subject->handleKeyRelease(KeyEvent("\b", QEvent::KeyRelease, Qt::Key_Backspace));
    QVERIFY(subject->preedit.isEmpty());
    QCOMPARE(inputMethodHost->commit, QString("ma"));
    QVERIFY(inputMethodHost->keyEvents.count() != 0);
    inputMethodHost->clear();
}

void Ut_MKeyboardHost::testDirectMode()
{
    QList<KeyEvent> testData;
    QList<Qt::Key> expectedKeys;

    testData << KeyEvent("\b", QEvent::KeyRelease, Qt::Key_Backspace)
             << KeyEvent("\r", QEvent::KeyRelease, Qt::Key_Return)
             << KeyEvent(" ", QEvent::KeyRelease, Qt::Key_Space);
    expectedKeys << Qt::Key_Backspace << Qt::Key_Return << Qt::Key_Space;
    QVERIFY(testData.count() == expectedKeys.count());

    subject->inputMethodMode = M::InputMethodModeDirect;

    for (int n = 0; n < testData.count(); ++n) {
        inputMethodHost->clear();
        subject->handleKeyRelease(testData.at(n));
        QCOMPARE(inputMethodHost->sendKeyEventCalls, 1);
        QVERIFY(inputMethodHost->keyEvents.first()->text() == testData.at(n).text());
        QVERIFY(inputMethodHost->keyEvents.first()->key() == expectedKeys.at(n));
        QVERIFY(inputMethodHost->keyEvents.first()->type() == QEvent::KeyRelease);
    }

    for (int n = 0; n < testData.count(); ++n) {
        inputMethodHost->clear();
        subject->handleKeyPress(KeyEvent(testData.at(n), QEvent::KeyPress));
        QCOMPARE(inputMethodHost->sendKeyEventCalls, 1);
        QVERIFY(inputMethodHost->keyEvents.first()->text() == testData.at(n).text());
        QVERIFY(inputMethodHost->keyEvents.first()->key() == expectedKeys.at(n));
        QVERIFY(inputMethodHost->keyEvents.first()->type() == QEvent::KeyPress);
    }
}

void Ut_MKeyboardHost::testNotCrash()
{
    //at least we should not crash
    subject->show();
    subject->hide();
    subject->reset();
    subject->setPreedit("string");
    subject->initializeInputEngine();
    subject->handleMouseClickOnPreedit(QPoint(0, 0), QRect(0, 0, 1, 1));
}

void Ut_MKeyboardHost::testErrorCorrectionOption()
{
    subject->show();
    MGConfItem config(InputMethodCorrectionSetting);
    config.set(QVariant(true));

    QVERIFY(subject->imCorrectionEngine != 0);
    //default error correction option is true;
    QVERIFY(subject->imCorrectionEngine->correctionEnabled() == true);
    QVERIFY(subject->correctionEnabled == true);

    bool originCorrection = false;
    if (!config.value().isNull())
        originCorrection = config.value().toBool();

    config.set(QVariant(false));
    QTest::qWait(100);
    QVERIFY(subject->imCorrectionEngine->correctionEnabled() == false);
    QVERIFY(subject->correctionEnabled == false);

    config.set(QVariant(originCorrection));
    subject->hide();
}

void Ut_MKeyboardHost::testAutoCaps()
{
    inputMethodHost->surroundingString = "Test string. You can using it!    ";
    inputMethodHost->autoCapitalizationEnabled_ = true;
    subject->correctionEnabled = true;
    inputMethodHost->contentType_ = M::FreeTextContentType;
    subject->show();

    inputMethodHost->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    inputMethodHost->cursorPos = 1;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    inputMethodHost->cursorPos = 12;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    inputMethodHost->cursorPos = 13;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);


    subject->hide();
    subject->show();

    inputMethodHost->cursorPos = 13;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    inputMethodHost->cursorPos = 16;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    inputMethodHost->cursorPos = 31;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    inputMethodHost->cursorPos = 33;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    inputMethodHost->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    // When autoCaps is on and shift is latched, any key input except shift and backspace (in an sepcial case)
    // will turn off shift.
    KeyEvent press("a", QEvent::KeyPress);
    KeyEvent release(press, QEvent::KeyRelease);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    // If there are some preedit, capitalization should be off.
    subject->preedit = "Test";
    inputMethodHost->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    subject->preedit = "";
    inputMethodHost->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    // The special case for backspace when autoCaps is on, that is cursor is at 0 position,
    // should not change the shift state.
    press = KeyEvent("", QEvent::KeyPress, Qt::Key_Backspace);
    release = KeyEvent(press, QEvent::KeyRelease);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    // If cursor is not at 0 position, backspace should also change the shift state.
    inputMethodHost->cursorPos = 2;
    subject->update();
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    // Test holding backspace with preedit.
    press = KeyEvent("", QEvent::KeyPress, Qt::Key_Backspace);
    release = KeyEvent(press, QEvent::KeyRelease);
    inputMethodHost->cursorPos = 18;
    subject->preedit = "You can use";
    // initial state: preedit("You can use"), shift state:latched, start holding backspace
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    // press and release backspace before timeout will only delete one character,
    subject->handleKeyPress(press);
    QVERIFY(subject->backspaceTimer.isActive());
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);
    QCOMPARE(subject->preedit, QString("You can us"));

    // but hold backspace longer than timeout, will delete the whole preedit.
    subject->handleKeyPress(press);
    int interval = subject->backspaceTimer.interval();
    QTest::qWait(interval / 2);
    QVERIFY(subject->backspaceTimer.isActive());
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);
    QTest::qWait((interval / 2) + 50);
    // final state: preedit(""), shift state:on, after holding backspace enough time.
    QVERIFY(subject->preedit.isEmpty());
    QVERIFY(!subject->backspaceTimer.isActive());
    inputMethodHost->cursorPos = 13;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    subject->hide();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);

    // Disable autoCaps
    inputMethodHost->autoCapitalizationEnabled_ = false;
    inputMethodHost->cursorPos = 0;
    subject->show();
    subject->update();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    // When shift is latched, any key input except shift will turn off shift.
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    press = KeyEvent("a", QEvent::KeyPress);
    release = KeyEvent(press, QEvent::KeyRelease);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    // Backspace will also change the shift state when cursor is at 0 position.
    inputMethodHost->cursorPos = 0;
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    press = KeyEvent("", QEvent::KeyPress, Qt::Key_Backspace);
    release = KeyEvent(press, QEvent::KeyRelease);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    // Test autocaps if autCaps flag is off from layout
    gAutoCapsEnabled = false;
    inputMethodHost->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);
    gAutoCapsEnabled = true;
}

void Ut_MKeyboardHost::testApplicationOrientationChanged()
{
    MAbstractInputMethod *im = subject;
    M::OrientationAngle angles[] = { M::Angle0, M::Angle90, M::Angle180, M::Angle270 };

    for (int i = 0; i < 5; ++i) {
        M::OrientationAngle currentAngle = angles[i % 4];
        im->handleAppOrientationChange(static_cast<int>(currentAngle));
        QTest::qWait(1500);
        QCOMPARE(currentAngle, MPlainWindow::instance()->orientationAngle());
    }
}

void Ut_MKeyboardHost::testCopyPaste()
{
    inputMethodHost->clear();

    subject->sendCopyPaste(InputMethodNoCopyPaste);
    QVERIFY(inputMethodHost->copyCalls == 0);
    QVERIFY(inputMethodHost->pasteCalls == 0);

    subject->sendCopyPaste(InputMethodCopy);
    QVERIFY(inputMethodHost->copyCalls == 1);
    QVERIFY(inputMethodHost->pasteCalls == 0);

    inputMethodHost->clear();
    subject->sendCopyPaste(InputMethodPaste);
    QVERIFY(inputMethodHost->copyCalls == 0);
    QVERIFY(inputMethodHost->pasteCalls == 1);
}

void Ut_MKeyboardHost::testPlusMinus()
{
    QString text = QChar(0xb1);
    inputMethodHost->sendKeyEventCalls = 0;
    KeyEvent press(text, QEvent::KeyPress, Qt::Key_plusminus, KeyEvent::NotSpecial);
    KeyEvent release(press, QEvent::KeyRelease);

    subject->update();
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release); // Should be ignored.

    QCOMPARE(inputMethodHost->sendKeyEventCalls, 2);
    QCOMPARE(inputMethodHost->keyEvents.first()->text(), text);
    QVERIFY(inputMethodHost->keyEvents.first()->key() == Qt::Key_plusminus);
    QCOMPARE(inputMethodHost->keyEvents.first()->type(), QEvent::KeyPress);
    QCOMPARE(inputMethodHost->keyEvents.at(1)->type(), QEvent::KeyRelease);
}

QRegion Ut_MKeyboardHost::region(RegionType type, int index)
{
    switch(type) {
    case ScreenRegion:
        return inputMethodHost->screenRegions.at(index);
        break;

    case InputMethodArea:
        return inputMethodHost->inputMethodAreas.at(index);
        break;
    }
}

void Ut_MKeyboardHost::testRegionSignals()
{
    // method call counts for region stuff
    int c1 = 0; // region updated
    int c2 = 0; // input method area updated

    subject->show();
    ++c1;
    ++c2;

    // We must immediately get non-empty region so that passthrough window
    // is made visible right before the animation starts.
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);
    QCOMPARE(region(ScreenRegion, c1 - 1), region(InputMethodArea, 0));
    QVERIFY(!region(ScreenRegion, c1 - 1).isEmpty());

    // We must get another region when the vkb is fully visible
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);
    ++c1;
    ++c2;
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);

    qDebug() << "Passthrough region: " << region(ScreenRegion, 1);
    qDebug() << "libmeegotouch region: " << region(InputMethodArea, 1);
    QVERIFY((region(ScreenRegion, c1 - 1) - region(InputMethodArea, 1)).isEmpty());

    // In normal input method mode there is no invisible handle with non-zero area
    const QRect zeroSizeInvisibleHandleRect(
        dynamic_cast<QGraphicsWidget*>(subject->sharedHandleArea->layout()->itemAt(0))
        ->geometry().toRect());
    QVERIFY(zeroSizeInvisibleHandleRect.isEmpty());

    // In direct mode an invisible handle is added on top of the keyboard
    // The test is disabed just like the invisible handle code itself (this
    // needs to be updated if it is enabled)
#if 0
    subject->vkbWidget->setInputMethodMode(M::InputMethodModeDirect);
    subject->sharedHandleArea->setInputMethodMode(M::InputMethodModeDirect);
    ++c1;
    ++c2;
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);

    const QRect invisibleHandleRect(
        dynamic_cast<QGraphicsWidget*>(subject->sharedHandleArea->layout()->itemAt(0))
        ->geometry().toRect());
    QVERIFY(!invisibleHandleRect.isEmpty());
    const QRegion invisibleHandleRegion(
        invisibleHandleRect.translated(0, (region(InputMethodArea, 0).boundingRect().top()
                                           - invisibleHandleRect.height())));

    QCOMPARE(region(ScreenRegion, c1 - 1), region(InputMethodArea, c2 - 1) + invisibleHandleRegion);

    subject->vkbWidget->setInputMethodMode(M::InputMethodModeNormal);
    subject->sharedHandleArea->setInputMethodMode(M::InputMethodModeNormal);
    ++c1;
    ++c2;
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);

    QCOMPARE(region(ScreenRegion, c1 - 1), region(InputMethodArea, c2 - 1));
    QCOMPARE(region(ScreenRegion, c1 - 1), region(InputMethodArea, c1 - 3));
#endif

    // In opaque mode, candidate widget has its own window, so no regions are sent to kbhost.
#ifndef DUI_IM_DISABLE_TRANSLUCENCY
    // Ditto for correction candidate widget
    subject->correctionHost->setCandidates((QStringList() << "abc" << "def"));
    subject->correctionHost->showCorrectionWidget();
    ++c1;
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);

    subject->correctionHost->hideCorrectionWidget();
    ++c1;
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);
    QCOMPARE(region(ScreenRegion, c1 - 1), region(InputMethodArea, c2 - 1));
#endif

    // But symbol view also changes input method area
    const int c1BeforeSymOpen = c1;
    subject->showSymbolView();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);

    ++c1;
    ++c2;
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);

    subject->symbolView->hideSymbolView();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);
    ++c1;
    ++c2;
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);

    // the same as before opening it
    QCOMPARE(region(ScreenRegion, c1BeforeSymOpen - 1), region(ScreenRegion, c1 - 1));

    // Hide the keyboard -> empty region and input method area
    subject->hide();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50); // really hidden after animation is finished
    ++c1;
    ++c2;
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);
    QCOMPARE(region(ScreenRegion, c1 - 1), QRegion());
    QCOMPARE(region(InputMethodArea, c2 - 1), QRegion());

    // Regions and rotation

    // Preparation: store 270deg-angle region obtained as safely as possible
    inputMethodHost->clear();

    rotateToAngle(M::Angle270);
    subject->show();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);

    QCOMPARE(inputMethodHost->setScreenRegionCalls, 2);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, 2);
    QCOMPARE(region(ScreenRegion, 1), region(InputMethodArea, 1));

    QRegion region270(region(ScreenRegion, 1));
    subject->hide();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);
    rotateToAngle(M::Angle0);

    QSignalSpy orientationSpy(window, SIGNAL(orientationChangeFinished(M::Orientation)));

    subject->show();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);

    inputMethodHost->clear();

    // Rotate three times repeatedly with long and short waits in between.  We
    // should end up with a region identical to that stored in region270.  The
    // wait times mimic user operations that have been found to cause a problem.
    window->setOrientationAngle(M::Angle90);
    QTest::qWait(800);
    qDebug() << "Orientations finished:" << orientationSpy.count();
    window->setOrientationAngle(M::Angle180);
    QTest::qWait(5);
    window->setOrientationAngle(M::Angle270);
    qDebug() << "Orientations finished:" << orientationSpy.count();
    qDebug() << "Waiting for rotation animation to finish...";
    QTest::qWait(SceneRotationTime); // wait until rotation animation is finished
    qDebug() << "Waiting for rotation animation to finish...done!";

    // Sanity checks
    qDebug() << "Orientations finished:" << orientationSpy.count();

    QCOMPARE(window->orientationAngle(), M::Angle270);
    QVERIFY(inputMethodHost->setScreenRegionCalls > 0);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, inputMethodHost->setScreenRegionCalls);

    qDebug() << "Region after animation is finished:" << subject->vkbWidget->region();

    // Now, is the region sane after those consequtive rotations?
    QCOMPARE(region(ScreenRegion, inputMethodHost->setScreenRegionCalls - 1),
             region(InputMethodArea, inputMethodHost->setInputMethodAreaCalls - 1));

    // Remove the next two lines when QCOMPARE is enabled
    qDebug() << "Actual region:" << region(ScreenRegion, inputMethodHost->setScreenRegionCalls - 1);
    qDebug() << "Expected region:" << region270;
    // This fails at the moment. libmeegotouch/Qt bug suspected.
    //QCOMPARE(region(ScreenRegion, spy.count() - 1), region270);
}

void Ut_MKeyboardHost::testSetState_data()
{
    QSet<MInputMethod::HandlerState> state;

    QTest::addColumn<QSet<MInputMethod::HandlerState> >("state");
    QTest::addColumn<int>("expectedCallCount");
    QTest::addColumn<MInputMethod::HandlerState>("expectedParameter");

    QTest::newRow("Empty") << state << 0 << MInputMethod::OnScreen;

    state.clear();
    state << MInputMethod::Hardware;
    QTest::newRow("Hardware") << state << 1 << MInputMethod::Hardware;

    state.clear();
    state << MInputMethod::OnScreen;
    QTest::newRow("OnScreen") << state << 0 << MInputMethod::OnScreen;

    state.clear();
    state << MInputMethod::Accessory;
    QTest::newRow("Accessory") << state << 1 << MInputMethod::Accessory;

    state.clear();
    state << MInputMethod::OnScreen << MInputMethod::Hardware;
    QTest::newRow("Sequence1") << state << 0 << MInputMethod::Hardware;

    state.clear();
    state << MInputMethod::Hardware << MInputMethod::Accessory;
    QTest::newRow("Sequence2") << state << 1 << MInputMethod::Hardware;
}

void Ut_MKeyboardHost::testSetState()
{
    QFETCH(QSet<MInputMethod::HandlerState>, state);
    QFETCH(int, expectedCallCount);
    QFETCH(MInputMethod::HandlerState, expectedParameter);

    qDebug() << "Probe state=" << state;

    gSetKeyboardStateCallCount = 0;
    subject->update();
    subject->setState(state);
    QCOMPARE(gSetKeyboardStateCallCount, expectedCallCount);
    if (gSetKeyboardStateCallCount) {
        QCOMPARE(gSetKeyboardStateParam, expectedParameter);
    }
}

void Ut_MKeyboardHost::testSetStateCombination()
{
    QSet<MInputMethod::HandlerState> state;

    gSetKeyboardStateCallCount = 0;
    state << MInputMethod::Hardware;
    subject->update();
    subject->setState(state);
    QCOMPARE(gSetKeyboardStateCallCount, 1);
    QCOMPARE(gSetKeyboardStateParam, MInputMethod::Hardware);
    subject->setState(state);
    QCOMPARE(gSetKeyboardStateCallCount, 1);
    QCOMPARE(gSetKeyboardStateParam, MInputMethod::Hardware);

    state.clear();
    state << MInputMethod::Hardware << MInputMethod::OnScreen;
    subject->setState(state);
    QCOMPARE(gSetKeyboardStateCallCount, 2);
    QCOMPARE(gSetKeyboardStateParam, MInputMethod::OnScreen);

    state.clear();
    state << MInputMethod::OnScreen;
    subject->setState(state);
    QCOMPARE(gSetKeyboardStateCallCount, 2);
    QCOMPARE(gSetKeyboardStateParam, MInputMethod::OnScreen);
    gSetKeyboardStateCallCount = 0;
}

void Ut_MKeyboardHost::testSymbolKeyClick()
{
    QVERIFY(subject->symbolView);

    // Symbol view is toggled by clicking the Sym button.

    // Initially symbol view is closed.
    QVERIFY(!subject->symbolView->isActive());
    bool symOpenExpected = false;

    for (int clicks = 1; clicks <= 3; ++clicks) {
        subject->handleSymbolKeyClick();

        const bool symOpen = subject->symbolView->isActive();
        symOpenExpected = !symOpenExpected;

        QCOMPARE(symOpen, symOpenExpected);
    }
}

void Ut_MKeyboardHost::testUpdateSymbolViewLevel()
{
    subject->show();
    QSet<MInputMethod::HandlerState> state;

    //hardware state
    QVERIFY(subject->hardwareKeyboard);
    QSignalSpy spy(subject->hardwareKeyboard, SIGNAL(shiftStateChanged()));

    state << MInputMethod::Hardware;
    subject->update();
    subject->setState(state);
    subject->symbolView->showSymbolView();
    QVERIFY(subject->symbolView->isActive());
    QCOMPARE(subject->symbolView->currentLevel(), 0);

    // test autocaps
    subject->autoCapsEnabled = true; // enable auto caps
    subject->hardwareKeyboard->setAutoCapitalization(true);
    QCOMPARE(subject->hardwareKeyboard->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    subject->hardwareKeyboard->setAutoCapitalization(false);
    QCOMPARE(subject->hardwareKeyboard->modifierState(Qt::ShiftModifier), ModifierClearState);

    spy.clear();
    //! first shift key press+release will latch the shift modifier, and then switch the symbolview level to 1
    // Note that the native modifier parameters are not correct but that doesn't matter for this test.
    subject->processKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, QString(""), false, 1, 0, 0);
    subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, Qt::ShiftModifier, QString(""), false, 1, 0, 0);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(subject->hardwareKeyboard->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    QCOMPARE(subject->symbolView->currentLevel(), 1);
    // second shift key press+release will lock the shift modifier, shift state changes but symbolview level stays 1
    subject->processKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, QString(""), false, 1, 0, 0);
    subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, Qt::ShiftModifier, QString(""), false, 1, 0, 0);
    QCOMPARE(spy.count(), 3);
    QCOMPARE(subject->hardwareKeyboard->modifierState(Qt::ShiftModifier), ModifierLockedState);
    QCOMPARE(subject->symbolView->currentLevel(), 1);
    //! third shift key press+release will clear the shift modifier, and switch the symbolview level back to 0
    subject->processKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, QString(""), false, 1, 0, 0);
    subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, Qt::ShiftModifier, QString(""), false, 1, 0, 0);
    QCOMPARE(spy.count(), 4);
    QCOMPARE(subject->hardwareKeyboard->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(subject->symbolView->currentLevel(), 0);
    subject->symbolView->hideSymbolView();

    //onscreen state
    QVERIFY(subject->vkbWidget);
    subject->autoCapsEnabled = false; // disable auto caps
    subject->vkbWidget->setShiftState(ModifierClearState);
    state.clear();
    state << MInputMethod::OnScreen;
    subject->setState(state);
    QSignalSpy spy1(subject->vkbWidget, SIGNAL(shiftLevelChanged()));

    QVERIFY(!subject->symbolView->isActive());
    subject->symbolView->showSymbolView();
    QVERIFY(subject->symbolView->isActive());
    QCOMPARE(subject->symbolView->currentLevel(), 0);
    QCOMPARE(subject->vkbWidget->shiftStatus(), ModifierClearState);

    spy1.clear();
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QCOMPARE(subject->vkbWidget->shiftStatus(), ModifierLatchedState);
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(subject->symbolView->currentLevel(), 1);

    subject->vkbWidget->setShiftState(ModifierLockedState);
    QCOMPARE(spy1.count(), 2);
    QCOMPARE(subject->vkbWidget->shiftStatus(), ModifierLockedState);
    QCOMPARE(subject->symbolView->currentLevel(), 1);

    subject->vkbWidget->setShiftState(ModifierClearState);
    QCOMPARE(spy1.count(), 3);
    QCOMPARE(subject->vkbWidget->shiftStatus(), ModifierClearState);
    QCOMPARE(subject->symbolView->currentLevel(), 0);

    subject->hide();
}

void Ut_MKeyboardHost::testKeyCycle_data()
{
    QTest::addColumn<QString>("preedit");

    QTest::newRow("no preedit") << "";
    QTest::newRow("with preedit") << "preedit";
}

void Ut_MKeyboardHost::testKeyCycle()
{
    QFETCH(QString, preedit);
    QString text = "123";
    KeyEvent event1(text,  QEvent::KeyRelease, Qt::Key_unknown, KeyEvent::CycleSet);
    KeyEvent event2("456", QEvent::KeyRelease, Qt::Key_unknown, KeyEvent::CycleSet);
    KeyEvent space ( " ",  QEvent::KeyRelease, Qt::Key_Space);
    KeyEvent invalid( "",  QEvent::KeyRelease, Qt::Key_unknown, KeyEvent::CycleSet);

    //this value must be greater that MultitapTime in the file mkeyboardhost.cpp
    const int MultitapTime = 2000;

    subject->update();
    inputMethodHost->preedit = "";
    inputMethodHost->commit = "";

    if (!preedit.isEmpty()) {
        subject->preedit = preedit;
    }

    for (int n = 0; n < event1.text().length(); ++n) {
        subject->handleKeyClick(event1);
        QCOMPARE(inputMethodHost->preedit, preedit + QString(text[n]));
        QCOMPARE(inputMethodHost->commit, QString(""));
    }
    QCOMPARE(inputMethodHost->preedit, preedit + QString(text[text.length() - 1]));
    QCOMPARE(inputMethodHost->commit, QString(""));

    subject->handleKeyClick(event2);
    QCOMPARE(inputMethodHost->commit, preedit + QString(text[text.length() - 1]));
    QCOMPARE(inputMethodHost->preedit, QString(event2.text()[0]));

    inputMethodHost->commit = "";

    QTest::qWait(MultitapTime);
    subject->handleKeyClick(event2);
    QCOMPARE(inputMethodHost->preedit, QString(event2.text()[0]));
    QCOMPARE(inputMethodHost->commit, QString(event2.text()[0]));

    inputMethodHost->commit  = "";
    inputMethodHost->preedit = "";

    subject->handleKeyClick(space);
    QCOMPARE(inputMethodHost->preedit, QString(""));
    QCOMPARE(inputMethodHost->commit, QString(event2.text()[0]) + " ");

    // Test cycle key autocommit timeout:
    inputMethodHost->commit = "";
    inputMethodHost->preedit = "";

    subject->handleKeyClick(event1);
    subject->handleKeyClick(event1);
    QCOMPARE(inputMethodHost->preedit, QString(event1.text()[1]));
    QCOMPARE(inputMethodHost->commit, QString(""));
    inputMethodHost->commit = "";
    inputMethodHost->preedit = "";
    QTest::qWait(MultitapTime);
    QCOMPARE(inputMethodHost->preedit, QString(""));
    QCOMPARE(inputMethodHost->commit, QString(event1.text()[1]));

    // Empty cycle string should not cause crash:
    subject->handleKeyClick(invalid);
}

void Ut_MKeyboardHost::testShiftState_data()
{
    QTest::addColumn<TestOpList>("operations");
    QTest::addColumn<ModifierState>("expectedShiftState");

    // Shift clicks without autocapitalization
    QTest::newRow("no click, no autocaps") << TestOpList() << ModifierClearState;
    QTest::newRow("click shift once") << (TestOpList() << ClickShift) << ModifierLatchedState;
    QTest::newRow("click shift twice") << (TestOpList() << ClickShift << ClickShift) << ModifierLockedState;
    QTest::newRow("click shift three times")
        << (TestOpList() << ClickShift << ClickShift << ClickShift) << ModifierClearState;

    // Shift clicks with autocapitalization
    QTest::newRow("autocaps on, no click")
        << (TestOpList() << TriggerAutoCaps) << ModifierLatchedState;
    QTest::newRow("autocaps on, click shift once")
        << (TestOpList() << TriggerAutoCaps << ClickShift) << ModifierClearState;
    QTest::newRow("autocaps on, click shift twice")
        << (TestOpList() << TriggerAutoCaps << ClickShift << ClickShift) << ModifierLatchedState;
    QTest::newRow("autocaps on, click shift three times")
        << (TestOpList() << TriggerAutoCaps << ClickShift << ClickShift << ClickShift) << ModifierLockedState;

    // Character clicks, how they clear shift

    QTest::newRow("manually latched shift, character click")
        << (TestOpList() << ClickShift << ClickCharacter) << ModifierClearState;
    QTest::newRow("automatically latched shift, character click")
        << (TestOpList() << TriggerAutoCaps << ClickCharacter) << ModifierClearState;
    QTest::newRow("locked shift, character click")
        << (TestOpList() << ClickShift << ClickShift << ClickCharacter) << ModifierLockedState;

    // Tricky cases

    // The shift+character here does not clear shift because shift was latched by the user and he/she had a reason for it.
    QTest::newRow("manually latched shift, shift+character click")
        << (TestOpList() << ClickShift << ClickCharacterWithShiftDown) << ModifierLatchedState;
    // Here user did not by him/herself latch shift. Any click should clear it.
    QTest::newRow("automatically latched shift, shift+character click")
        << (TestOpList() << TriggerAutoCaps << ClickCharacterWithShiftDown) << ModifierClearState;
}

void Ut_MKeyboardHost::testShiftState()
{
    QFETCH(TestOpList, operations);
    QFETCH(ModifierState, expectedShiftState);

    const KeyEvent shiftClickEvent(QString(), QEvent::KeyRelease, Qt::Key_Shift);

    // We don't use Qt::ShiftModifier for shift state since it does not differentiate between
    // latched+char and held+char. Vkb sets it in both situations.
    const KeyEvent characterClickEvent("a", QEvent::KeyRelease);

    foreach (TestOperation op, operations) {
        switch (op) {
        case ClickShift:
            subject->handleKeyClick(shiftClickEvent);
            break;
        case ClickCharacter:
            subject->handleKeyClick(characterClickEvent);
            break;
        case ClickCharacterWithShiftDown:
            subject->handleKeyPress(shiftClickEvent);
            subject->handleKeyClick(characterClickEvent);
            subject->handleKeyRelease(shiftClickEvent);
            break;
        case TriggerAutoCaps:
            triggerAutoCaps();
            break;
        default:
            QFAIL("invalid test operation");
            return;
        }
    }

    QCOMPARE(subject->vkbWidget->shiftStatus(), expectedShiftState);
}

void Ut_MKeyboardHost::testCommitPreeditOnStateChange()
{
    const QString text("fish");
    subject->setPreedit(text);
    QSet<MInputMethod::HandlerState> state;
    state << MInputMethod::Hardware;
    subject->setState(state);
    QCOMPARE(inputMethodHost->commit, text);
}

void Ut_MKeyboardHost::testLayoutMenuKeyClick_data()
{
    QTest::addColumn<ModifierState>("shiftState");

    QTest::newRow("Clear")   << ModifierClearState;
    QTest::newRow("Latched") << ModifierLatchedState;
    QTest::newRow("Locked")  << ModifierLockedState;
}

void Ut_MKeyboardHost::testLayoutMenuKeyClick()
{
    //menu key should not toggle shift key
    QFETCH(ModifierState, shiftState);
    KeyEvent menuClickEvent(QString(), QEvent::KeyRelease, Qt::Key_unknown, KeyEvent::LayoutMenu);

    subject->vkbWidget->setShiftState(shiftState);
    subject->handleGeneralKeyClick(menuClickEvent);
    QCOMPARE(subject->vkbWidget->shiftStatus(), shiftState);
}

void Ut_MKeyboardHost::testShiftStateOnFocusChanged_data()
{
    QTest::addColumn<MInputMethod::HandlerState>("state");
    QTest::addColumn<ModifierState>("initialShiftState");
    QTest::addColumn<QString>("surroundingString");
    QTest::addColumn<bool>("autoCapitalizationEnabled");
    QTest::addColumn<int>("cursorPosition");
    QTest::addColumn<ModifierState>("expectedShiftState");

    // corsor is at the begining.
    QTest::newRow("screen lowercase") << MInputMethod::OnScreen << ModifierClearState << QString("Test. ")
                                      << true << 0 << ModifierLatchedState;
    QTest::newRow("screen latched") << MInputMethod::OnScreen << ModifierLatchedState << QString("Test. ")
                                    << true << 2 << ModifierClearState;
    // cursor is right after a dot.
    QTest::newRow("screen latched") << MInputMethod::OnScreen << ModifierLatchedState << QString("Test. ")
                                    << true << 6 << ModifierLatchedState;
    QTest::newRow("screen latched") << MInputMethod::OnScreen << ModifierLatchedState << QString("Test. ")
                                    << true << 2  << ModifierClearState;
    QTest::newRow("screen locked") << MInputMethod::OnScreen << ModifierLockedState << QString("Test. ")
                                   << true << 0 << ModifierLockedState;
    QTest::newRow("screen locked") << MInputMethod::OnScreen << ModifierLockedState << QString("Test. ")
                                   << true << 2 << ModifierLockedState;
    // text entry disable autocaps.
    QTest::newRow("screen locked") << MInputMethod::OnScreen << ModifierLatchedState << QString("Test. ")
                                   << false << 0 << ModifierClearState;
}

void Ut_MKeyboardHost::testShiftStateOnFocusChanged()
{
    // all temporary shift state (not capslock) should be reset when
    // focus is changed, and new shift state depends on autocaps
    QFETCH(MInputMethod::HandlerState, state);
    QFETCH(ModifierState, initialShiftState);
    QFETCH(QString, surroundingString);
    QFETCH(bool, autoCapitalizationEnabled);
    QFETCH(int, cursorPosition);
    QFETCH(ModifierState, expectedShiftState);

    QSet<MInputMethod::HandlerState> set;
    set << state;
    subject->setState(set);

    subject->vkbWidget->setShiftState(initialShiftState);

    subject->handleFocusChange(true);
    inputMethodHost->surroundingString = surroundingString;
    inputMethodHost->autoCapitalizationEnabled_ = autoCapitalizationEnabled;
    inputMethodHost->cursorPos = cursorPosition;
    subject->update();

    QCOMPARE(subject->vkbWidget->shiftStatus(), expectedShiftState);

}

void Ut_MKeyboardHost::testShiftStateOnLayoutChanged_data()
{
    QTest::addColumn<QString>("surroundingString");
    QTest::addColumn<bool>("autoCapitalizationEnabled");
    QTest::addColumn<int>("cursorPosition");
    QTest::addColumn<QString>("layout");
    QTest::addColumn<bool>("layoutAutoCapitalization");
    QTest::addColumn<ModifierState>("initialShiftState");
    QTest::addColumn<ModifierState>("expectedShiftState");

    // manually set shift state to shift on, changing layout will turn it back to lowercase according autocaps
    QTest::newRow("screen lowercase") << QString("Test. ") << true << 2 << QString("en_gb")
                                      << true << ModifierLatchedState << ModifierClearState;

    // manually set shift state to shift on, changing layout will keep it according autocaps
    QTest::newRow("screen latched") << QString("Test. ") << true << 0 << QString("fi")
                                    << true << ModifierLatchedState << ModifierLatchedState;

    // manually set shift state to lowercase, changing layout will turn it back to shift on according autocaps
    QTest::newRow("screen latched") << QString("Test. ") << true << 0 << QString("fr")
                                    << true << ModifierClearState << ModifierLatchedState;

    // manually set shift state to shift locked, changing layout won't change shift state
    QTest::newRow("screen locked") << QString("Test. ") << true << 0 << QString("en_us")
                                   << true << ModifierLockedState << ModifierLockedState;

    // manually set shift state to shift on, changing layout will change shift to lowercase because
    // layout disable autocaps.
    QTest::newRow("screen latched") << QString("Test. ") << true << 0 << QString("ar")
                                   << false << ModifierLatchedState << ModifierClearState;

    // manually set shift state to shift locked, changing layout won't change shift state even if
    // layout disable autocaps.
    QTest::newRow("screen latched") << QString("Test. ") << true << 0 << QString("ar")
                                   << false << ModifierLockedState << ModifierLockedState;
}

void Ut_MKeyboardHost::testShiftStateOnLayoutChanged()
{
    // all temporary shift state (not capslock) should be reset when
    // focus is changed, and new shift state depends on autocaps
    QFETCH(QString, surroundingString);
    QFETCH(bool, autoCapitalizationEnabled);
    QFETCH(int, cursorPosition);
    QFETCH(QString, layout);
    QFETCH(bool, layoutAutoCapitalization);
    QFETCH(ModifierState, initialShiftState);
    QFETCH(ModifierState, expectedShiftState);

    QSet<MInputMethod::HandlerState> set;
    set << MInputMethod::OnScreen;
    subject->setState(set);

    inputMethodHost->surroundingString = surroundingString;
    inputMethodHost->autoCapitalizationEnabled_ = autoCapitalizationEnabled;
    inputMethodHost->cursorPos = cursorPosition;
    subject->vkbWidget->setShiftState(initialShiftState);

    gAutoCapsEnabled = layoutAutoCapitalization;
    subject->handleVirtualKeyboardLayoutChanged(layout);

    QCOMPARE(subject->vkbWidget->shiftStatus(), expectedShiftState);
}

void Ut_MKeyboardHost::rotateToAngle(M::OrientationAngle angle)
{
    subject->handleAppOrientationChange(angle);
    QTest::qWait(SceneRotationTime); // wait until rotation animation is finished
}

void Ut_MKeyboardHost::triggerAutoCaps()
{
    // Set necessary conditions for triggering auto capitalization
    gAutoCapsEnabled = true;
    subject->preedit.clear();
    subject->cursorPos = 0;
    inputMethodHost->contentType_ = M::FreeTextContentType;
    inputMethodHost->autoCapitalizationEnabled_ = true;

    // Update
    subject->updateAutoCapitalization();
    QVERIFY(subject->autoCapsTriggered);
}

void Ut_MKeyboardHost::testToolbar()
{
    const QString toolbarName1 = QCoreApplication::applicationDirPath() + "/toolbar1.xml";
    const QString toolbarName2 = QCoreApplication::applicationDirPath() + "/toolbar2.xml";

    QVERIFY2(QFile(toolbarName1).exists(), "toolbar1.xml does not exist");
    QVERIFY2(QFile(toolbarName2).exists(), "toolbar2.xml does not exist");

    QSharedPointer<MToolbarData> toolbar1(new MToolbarData);
    QSharedPointer<MToolbarData> toolbar2(new MToolbarData);
    QSharedPointer<MToolbarData> nothing;
    bool ok;

    ok = toolbar1->loadToolbarXml(toolbarName1);
    QVERIFY2(ok, "toolbar1.xml was not loaded correctly");

    ok = toolbar2->loadToolbarXml(toolbarName2);
    QVERIFY2(ok, "toolbar2.xml was not loaded correctly");

    // verify is showToolbarWidget was called
    subject->setToolbar(toolbar2);
    QCOMPARE(gShowToolbarWidgetCalls, 1);
    QCOMPARE(gHideToolbarWidgetCalls, 0);

    subject->setToolbar(toolbar1);
    // verify if hideToolbarWidget was called
    QCOMPARE(gShowToolbarWidgetCalls, 1);
    QCOMPARE(gHideToolbarWidgetCalls, 1);

    subject->setToolbar(toolbar2);
    gShowToolbarWidgetCalls = 0;
    gHideToolbarWidgetCalls = 0;

    subject->setToolbar(nothing);
    QCOMPARE(gShowToolbarWidgetCalls, 0);
    QCOMPARE(gHideToolbarWidgetCalls, 1);
}

void Ut_MKeyboardHost::testHandleHwKeyboardStateChanged_data()
{
    QTest::addColumn<QString>("xkbLayout");
    QTest::addColumn<QString>("xkbVariant");
    QTest::addColumn<int>("shiftClickedCount");
    QTest::addColumn<int>("fnClickedCount");
    QTest::addColumn<MInputMethod::InputModeIndicator>("expectIndicator");
    QTest::addColumn<bool>("notificationShowCalled");
    QTest::addColumn<int>("deadKeyCharacterCode");

    QTest::newRow("English clear indicator") << QString("us") << QString("") << 0
                                             << 0 << MInputMethod::LatinLowerIndicator
                                             << false << 0;
    QTest::newRow("English shift latched indicator") << QString("us") << QString("") << 1
                                                     << 0 << MInputMethod::LatinUpperIndicator
                                                     << false << 0;
    QTest::newRow("English shift locked indicator") << QString("us") << QString("") << 2
                                                    << 0 << MInputMethod::LatinLockedIndicator
                                                    << true << 0;
    QTest::newRow("English fn latched indicator") << QString("us") << QString("") << 0
                                                  << 1 << MInputMethod::NumAndSymLatchedIndicator
                                                  << false << 0;
    QTest::newRow("English fn locked indicator") << QString("us") << QString("") << 0
                                                 << 2 << MInputMethod::NumAndSymLockedIndicator
                                                 << true << 0;
    QTest::newRow("Cyrillic clear indicator") << QString("ru") << QString("cyrillic") << 0
                                              << 0 << MInputMethod::CyrillicLowerIndicator
                                              << false << 0;
    QTest::newRow("Cyrillic shift latched indicator") << QString("ru") << QString("cyrillic") << 1
                                                      << 0 << MInputMethod::CyrillicUpperIndicator
                                                      << false << 0;
    QTest::newRow("Cyrillic shift locked indicator") << QString("ru") << QString("cyrillic") << 2
                                                     << 0 << MInputMethod::CyrillicLockedIndicator
                                                     << true << 0;
    QTest::newRow("Latin clear indicator") << QString("ru") << QString("latin") << 0
                                              << 0 << MInputMethod::LatinLowerIndicator
                                              << false << 0;
    QTest::newRow("Latin shift latched indicator") << QString("ru") << QString("latin") << 1
                                                   << 0 << MInputMethod::LatinUpperIndicator
                                                   << false << 0;
    QTest::newRow("Latin shift locked indicator") << QString("us") << QString("latin") << 2
                                                  << 0 << MInputMethod::LatinLockedIndicator
                                                  << true << 0;
    QTest::newRow("Arabic indicator") << QString("ara") << QString("") << 0
                                      << 0 << MInputMethod::ArabicIndicator
                                      << false << 0;
    QTest::newRow("DeadKeyAcute") << QString("br") << QString("") << 2 << 0
                                  << MInputMethod::DeadKeyAcuteIndicator << true << 0x00b4;
    QTest::newRow("DeadKeyCaron") << QString("sk") << QString("") << 2 << 0
                                  << MInputMethod::DeadKeyCaronIndicator << true << 0x02c7;
    QTest::newRow("DeadKeyCircumflex") << QString("br") << QString("") << 2 << 0
                                       << MInputMethod::DeadKeyCircumflexIndicator
                                       << true << 0x005e;
    QTest::newRow("DeadKeyDiaeresis") << QString("fr") << QString("") << 2 << 0
                                      << MInputMethod::DeadKeyDiaeresisIndicator << true << 0x00a8;
    QTest::newRow("DeadKeyGrave") << QString("br") << QString("") << 2 << 0
                                  << MInputMethod::DeadKeyGraveIndicator << true << 0x0060;
    QTest::newRow("DeadKeyTilde") << QString("br") << QString("") << 2 << 0
                                  << MInputMethod::DeadKeyTildeIndicator << true << 0x007e;
}

void Ut_MKeyboardHost::testHandleHwKeyboardStateChanged()
{
    QFETCH(QString, xkbLayout);
    QFETCH(QString, xkbVariant);
    QFETCH(int, shiftClickedCount);
    QFETCH(int, fnClickedCount);
    QFETCH(MInputMethod::InputModeIndicator, expectIndicator);
    QFETCH(bool, notificationShowCalled);
    QFETCH(int, deadKeyCharacterCode);

    LayoutsManager::instance().setXkbMap(xkbLayout, xkbVariant);
    QSet<MInputMethod::HandlerState> states;
    states << MInputMethod::Hardware;
    subject->setState(states);
    subject->handleFocusChange(true);
    gShowLockOnInfoBannerCallCount = 0;

    for (int i = 0; i < shiftClickedCount;  i++) {
        subject->processKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, QString(""), false, 1, 0, 0);
        subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, Qt::ShiftModifier, QString(""), false, 1, 0, 0);
    }

    for (int i = 0; i < fnClickedCount;  i++) {
        subject->processKeyEvent(QEvent::KeyPress, FnLevelKey, Qt::NoModifier, QString(""), false, 1, 0, 0);
        subject->processKeyEvent(QEvent::KeyRelease, FnLevelKey, FnLevelModifier, QString(""), false, 1, 0, 0);
    }

    if (deadKeyCharacterCode) {
        MGConfItem layoutConfig(XkbLayoutSettingName);
        layoutConfig.set(xkbLayout);
        MGConfItem variantConfig(XkbVariantSettingName);
        variantConfig.set(xkbVariant);

        subject->processKeyEvent(QEvent::KeyPress, Qt::Key_unknown, Qt::NoModifier,
                                 QString(QChar(deadKeyCharacterCode)), false, 1, 0, 0);
        subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_unknown, Qt::NoModifier,
                                 QString(QChar(deadKeyCharacterCode)), false, 1, 0, 0);
    }

    QCOMPARE(inputMethodHost->indicator, expectIndicator);
    QCOMPARE(gShowLockOnInfoBannerCallCount, notificationShowCalled ? 1 : 0);

    if (deadKeyCharacterCode) {
        // When state is changed from locked -> dead key -> locked, we don't want a notification
        subject->processKeyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier,
                                 QString("A"), false, 1, 0, 0);
        subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier,
                                 QString("A"), false, 1, 0, 0);
        QCOMPARE(inputMethodHost->indicator, MInputMethod::LatinLockedIndicator);
        QCOMPARE(gShowLockOnInfoBannerCallCount, 1);
    }
}

void Ut_MKeyboardHost::testUserHide()
{
    subject->show();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);

    QVERIFY(subject->vkbWidget->isFullyVisible());
    subject->userHide();
    QVERIFY(!subject->vkbWidget->isFullyVisible());
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);
    QVERIFY(!subject->vkbWidget->isVisible());

    QVERIFY(inputMethodHost->setScreenRegionCalls > 0);
    QVERIFY(region(ScreenRegion, inputMethodHost->setScreenRegionCalls - 1).isEmpty());
}

void Ut_MKeyboardHost::testWYTIWYSErrorCorrection()
{
    // Pre-edit must not be auto-corrected

    subject->show();
    MGConfItem config(InputMethodCorrectionSetting);
    config.set(QVariant(true));

    if (subject->imCorrectionEngine) {
        MImEngineWordsInterfaceFactory::instance()->deleteEngine(subject->imCorrectionEngine);
    }

    DummyDriverMkh *engine(new DummyDriverMkh);
    subject->imCorrectionEngine = engine;
    QStringList candidates;
    candidates << "a" << "c" << "d";
    engine->setCandidates(candidates);
    engine->setSuggestedCandidateIndexReturnValue(1);

    subject->handleKeyClick(KeyEvent("d"));
    QCOMPARE(inputMethodHost->preedit, QString("d"));

    subject->hide();
    delete engine;
    subject->imCorrectionEngine = 0;
}

const struct {
    const char *str;
    Qt::Key key;
}
testEvents[] = {
    { "k", Qt::Key_K },
    { "+-", Qt::Key_plusminus },
    { " ", Qt::Key_Space },
    { "s", Qt::Key_S },
    { "\b", Qt::Key_Backspace },
    { 0, Qt::Key_unknown }
};

void Ut_MKeyboardHost::testSignals(M::InputMethodMode inputMethodMode)
{
    subject->inputMethodMode = inputMethodMode;

    inputMethodHost->clear();
    for(int i = 0; testEvents[i].str != 0; ++i ) {
        if (testEvents[i].key == Qt::Key_Backspace) {
            QSKIP("Backspace key is known to be broken", SkipSingle);
        }
        subject->handleKeyPress(KeyEvent(testEvents[i].str, QEvent::KeyPress,
                                         testEvents[i].key) );
        QCOMPARE(inputMethodHost->sendKeyEventCalls, 2*i+1);
        subject->handleKeyRelease(KeyEvent(testEvents[i].str, QEvent::KeyRelease,
                                         testEvents[i].key) );
        QCOMPARE(inputMethodHost->sendKeyEventCalls, 2*i+2);
    }
}

void Ut_MKeyboardHost::testSignalsInNormalMode()
{
    testSignals(M::InputMethodModeNormal);
}

void Ut_MKeyboardHost::testSignalsInDirectMode()
{
    testSignals(M::InputMethodModeDirect);
}

void Ut_MKeyboardHost::testShowLanguageNotification_data()
{
    QTest::addColumn<MInputMethod::HandlerState>("state");
    QTest::addColumn<int>("expectedCallCount");

    QTest::newRow("OnScreen") << MInputMethod::OnScreen << 1;
    QTest::newRow("Hardware") << MInputMethod::Hardware << 0;
    QTest::newRow("Accessory") << MInputMethod::Accessory << 0;
}

void Ut_MKeyboardHost::testShowLanguageNotification()
{
    QFETCH(MInputMethod::HandlerState, state);
    QFETCH(int, expectedCallCount);
    QSet<MInputMethod::HandlerState> states;

    states << state;

    gRequestLanguageNotificationCallCount = 0;
    subject->update();
    subject->setState(states);
    subject->showLanguageNotification();
    QCOMPARE(gRequestLanguageNotificationCallCount, expectedCallCount);
}

QTEST_APPLESS_MAIN(Ut_MKeyboardHost);

