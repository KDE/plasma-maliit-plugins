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



#include <mimcorrectioncandidatewidget.h>
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
#include "minputcontextstubconnection.h"
#include "ut_mkeyboardhost.h"
#include "utils.h"

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
    const QString InputMethodCorrectionSetting("/meegotouch/inputmethods/correctionenabled");
    const QString InputMethodCorrectionEngine("/meegotouch/inputmethods/correctionengine");
    int gSetKeyboardStateCallCount = 0;
    MIMHandlerState gSetKeyboardStateParam = OnScreen;
    const int SceneRotationTime = 1400; // in ms
    bool gAutoCapsEnabled = true;

    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";

    int gShowToolbarWidgetCalls = 0;
    int gHideToolbarWidgetCalls = 0;

    const Qt::Key FnLevelKey = Qt::Key_AltGr;
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;

    MInputMethodBase::InputModeIndicator gInputMethodIndicator = MInputMethodBase::NoIndicator;

    int gShowLockOnInfoBannerCallCount = 0;
    int gHideLockOnInfoBannerCallCount = 0;
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

Q_DECLARE_METATYPE(QSet<MIMHandlerState>)
Q_DECLARE_METATYPE(MIMHandlerState)
Q_DECLARE_METATYPE(ModifierState)
Q_DECLARE_METATYPE(Ut_MKeyboardHost::TestOpList)
Q_DECLARE_METATYPE(MInputMethodBase::InputModeIndicator)

static void waitForSignal(const QObject* object, const char* signal, int timeout = 500)
{
    QEventLoop eventLoop;
    QObject::connect(object, signal, &eventLoop, SLOT(quit()));
    QTimer::singleShot(timeout, &eventLoop, SLOT(quit()));
    eventLoop.exec();
}


// Stubbing..................................................................

void MVirtualKeyboard::setKeyboardState(MIMHandlerState state)
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

void MInputMethodBase::sendInputModeIndicator(MInputMethodBase::InputModeIndicator indicatorState)
{
    gInputMethodIndicator = indicatorState;
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
    app = new MApplication(argc, app_name);
    inputContext = new MInputContextStubConnection;
    window = new MPlainWindow;

    MGConfItem(MultitouchSettings).set(true);

    qRegisterMetaType<M::Orientation>("M::Orientation");
    qRegisterMetaType<TestOpList>("TestOpList");
}

void Ut_MKeyboardHost::cleanupTestCase()
{
    delete window;
    window = 0;
    delete inputContext;
    inputContext = 0;
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

    subject = new MKeyboardHost(inputContext, 0);
    inputContext->clear();
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
    QCOMPARE(inputContext->preedit, QString("a"));

    subject->handleKeyClick(KeyEvent(" ", QEvent::KeyRelease, Qt::Key_Space));
    QVERIFY(subject->preedit.isEmpty());
    QCOMPARE(inputContext->commit, QString("a "));
    inputContext->clear();

    subject->handleKeyClick(KeyEvent("a"));
    subject->handleKeyClick(KeyEvent("\n", QEvent::KeyRelease, Qt::Key_Return));
    QVERIFY(subject->preedit.isEmpty());
    QCOMPARE(inputContext->commit, QString("a\n"));
    inputContext->clear();

    subject->handleKeyClick(KeyEvent("a"));
    subject->handleKeyPress(KeyEvent("\b", QEvent::KeyPress, Qt::Key_Backspace));
    subject->handleKeyRelease(KeyEvent("\b", QEvent::KeyRelease, Qt::Key_Backspace));
    QVERIFY(subject->preedit.isEmpty());
    inputContext->clear();

    // turn off error correction
    config.set(QVariant(false));
    QTest::qWait(100);
    QCOMPARE(subject->imCorrectionEngine->correctionEnabled(), false);
    QCOMPARE(subject->correctionEnabled, false);

    subject->handleKeyClick(KeyEvent("m"));
    subject->handleKeyClick(KeyEvent("a"));
    QCOMPARE(inputContext->commit, QString("ma"));
    QVERIFY(subject->preedit.isEmpty());

    // turn on error correction
    config.set(QVariant(true));
    QTest::qWait(100);
    QCOMPARE(subject->imCorrectionEngine->correctionEnabled(), true);
    QCOMPARE(subject->correctionEnabled, true);

    subject->handleKeyPress(KeyEvent("\b", QEvent::KeyPress, Qt::Key_Backspace));
    subject->handleKeyRelease(KeyEvent("\b", QEvent::KeyRelease, Qt::Key_Backspace));
    QVERIFY(subject->preedit.isEmpty());
    QCOMPARE(inputContext->commit, QString("ma"));
    QVERIFY(inputContext->keyEvents.count() != 0);
    inputContext->clear();
}

void Ut_MKeyboardHost::testDirectMode()
{
    QList<KeyEvent> testData;
    QList<Qt::Key> expectedKeys;

    testData << KeyEvent("\b", QEvent::KeyRelease, Qt::Key_Backspace)
             << KeyEvent("\n", QEvent::KeyRelease, Qt::Key_Return)
             << KeyEvent(" ", QEvent::KeyRelease, Qt::Key_Space);
    expectedKeys << Qt::Key_Backspace << Qt::Key_Return << Qt::Key_Space;
    QVERIFY(testData.count() == expectedKeys.count());

    subject->inputMethodMode = M::InputMethodModeDirect;

    for (int n = 0; n < testData.count(); ++n) {
        inputContext->clear();
        subject->handleKeyRelease(testData.at(n));
        QCOMPARE(inputContext->sendKeyEventCalls, 1);
        QVERIFY(inputContext->keyEvents.first()->text() == testData.at(n).text());
        QVERIFY(inputContext->keyEvents.first()->key() == expectedKeys.at(n));
        QVERIFY(inputContext->keyEvents.first()->type() == QEvent::KeyRelease);
    }

    for (int n = 0; n < testData.count(); ++n) {
        inputContext->clear();
        subject->handleKeyPress(KeyEvent(testData.at(n), QEvent::KeyPress));
        QCOMPARE(inputContext->sendKeyEventCalls, 1);
        QVERIFY(inputContext->keyEvents.first()->text() == testData.at(n).text());
        QVERIFY(inputContext->keyEvents.first()->key() == expectedKeys.at(n));
        QVERIFY(inputContext->keyEvents.first()->type() == QEvent::KeyPress);
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
    subject->mouseClickedOnPreedit(QPoint(0, 0), QRect(0, 0, 1, 1));
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
    inputContext->surroundingString = "Test string. You can using it!    ";
    inputContext->autoCapitalizationEnabled_ = true;
    subject->correctionEnabled = true;
    inputContext->contentType_ = M::FreeTextContentType;
    subject->show();

    inputContext->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    inputContext->cursorPos = 1;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    inputContext->cursorPos = 12;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    inputContext->cursorPos = 13;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);


    subject->hide();
    subject->show();

    inputContext->cursorPos = 13;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    inputContext->cursorPos = 16;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    inputContext->cursorPos = 31;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    inputContext->cursorPos = 33;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    inputContext->cursorPos = 0;
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
    inputContext->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    subject->preedit = "";
    inputContext->cursorPos = 0;
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
    inputContext->cursorPos = 2;
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
    inputContext->cursorPos = 18;
    subject->preedit = "You can use";
    // initial state: preedit("You can use"), shift state:latched, start holding backspace
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    // press and release backspace before timeout will only delete one character,
    subject->handleKeyPress(press);
    QVERIFY(subject->backSpaceTimer.isActive());
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);
    QCOMPARE(subject->preedit, QString("You can us"));

    // but hold backspace longer than timeout, will delete the whole preedit.
    subject->handleKeyPress(press);
    int interval = subject->backSpaceTimer.interval();
    QTest::qWait(interval / 2);
    QVERIFY(subject->backSpaceTimer.isActive());
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);
    QTest::qWait((interval / 2) + 50);
    // final state: preedit(""), shift state:on, after holding backspace enough time.
    QVERIFY(subject->preedit.isEmpty());
    QVERIFY(!subject->backSpaceTimer.isActive());
    inputContext->cursorPos = 13;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    subject->hide();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);

    // Disable autoCaps
    inputContext->autoCapitalizationEnabled_ = false;
    inputContext->cursorPos = 0;
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
    inputContext->cursorPos = 0;
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
    inputContext->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);
    gAutoCapsEnabled = true;
}

void Ut_MKeyboardHost::testApplicationOrientationChanged()
{
    MInputMethodBase *im = subject;
    M::OrientationAngle angles[] = { M::Angle0, M::Angle90, M::Angle180, M::Angle270 };

    for (int i = 0; i < 5; ++i) {
        M::OrientationAngle currentAngle = angles[i % 4];
        im->appOrientationChanged(static_cast<int>(currentAngle));
        QTest::qWait(1500);
        QCOMPARE(currentAngle, MPlainWindow::instance()->orientationAngle());
    }
}

void Ut_MKeyboardHost::testCopyPaste()
{
    inputContext->clear();

    subject->sendCopyPaste(InputMethodNoCopyPaste);
    QVERIFY(inputContext->copyCalls == 0);
    QVERIFY(inputContext->pasteCalls == 0);

    subject->sendCopyPaste(InputMethodCopy);
    QVERIFY(inputContext->copyCalls == 1);
    QVERIFY(inputContext->pasteCalls == 0);

    inputContext->clear();
    subject->sendCopyPaste(InputMethodPaste);
    QVERIFY(inputContext->copyCalls == 0);
    QVERIFY(inputContext->pasteCalls == 1);
}

void Ut_MKeyboardHost::testPlusMinus()
{
    QString text = QChar(0xb1);
    inputContext->sendKeyEventCalls = 0;
    KeyEvent press(text, QEvent::KeyPress, Qt::Key_plusminus, KeyEvent::NotSpecial);
    KeyEvent release(press, QEvent::KeyRelease);

    subject->update();
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release); // Should be ignored.

    QCOMPARE(inputContext->sendKeyEventCalls, 2);
    QCOMPARE(inputContext->keyEvents.first()->text(), text);
    QVERIFY(inputContext->keyEvents.first()->key() == Qt::Key_plusminus);
    QCOMPARE(inputContext->keyEvents.first()->type(), QEvent::KeyPress);
    QCOMPARE(inputContext->keyEvents.at(1)->type(), QEvent::KeyRelease);
}

static QRegion region(const QSignalSpy &spy, int index)
{
    return spy.at(index).at(0).value<QRegion>();
}

void Ut_MKeyboardHost::testRegionSignals()
{
    qRegisterMetaType<QRegion>("QRegion");
    QSignalSpy spy(subject, SIGNAL(regionUpdated(QRegion)));
    QSignalSpy spy2(subject, SIGNAL(inputMethodAreaUpdated(QRegion)));

    // Counts for signal spy
    int c1 = 0;
    int c2 = 0;

    subject->show();
    ++c1;
    ++c2;
    // We must immediately get non-empty region so that passthrough window
    // is made visible right before the animation starts.
    QCOMPARE(spy.count(), c1);
    QCOMPARE(spy2.count(), c2);

    QCOMPARE(region(spy, c1 - 1), region(spy2, 0));
    QVERIFY(!region(spy, c1 - 1).isEmpty());

    // We must get another region when the vkb is fully visible
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);
    ++c1;
    ++c2;
    QCOMPARE(spy.count(), c1);
    QCOMPARE(spy2.count(), c2);
    qDebug() << "Passthrough region: " << region(spy, 1);
    qDebug() << "libmeegotouch region: " << region(spy2, 1);
    QVERIFY((region(spy, c1 - 1) - region(spy2, 1)).isEmpty());

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
    QCOMPARE(spy.count(), c1);
    QCOMPARE(spy2.count(), c2);

    const QRect invisibleHandleRect(
        dynamic_cast<QGraphicsWidget*>(subject->sharedHandleArea->layout()->itemAt(0))
        ->geometry().toRect());
    QVERIFY(!invisibleHandleRect.isEmpty());
    const QRegion invisibleHandleRegion(
        invisibleHandleRect.translated(0, (region(spy2, 0).boundingRect().top()
                                           - invisibleHandleRect.height())));

    QCOMPARE(region(spy, c1 - 1), region(spy2, c2 - 1) + invisibleHandleRegion);

    subject->vkbWidget->setInputMethodMode(M::InputMethodModeNormal);
    subject->sharedHandleArea->setInputMethodMode(M::InputMethodModeNormal);
    ++c1;
    ++c2;
    QCOMPARE(spy.count(), c1);
    QCOMPARE(spy2.count(), c2);

    QCOMPARE(region(spy, c1 - 1), region(spy2, c2 - 1));
    QCOMPARE(region(spy, c1 - 1), region(spy2, c1 - 3));
#endif

    // In opaque mode, candidate widget has its own window, so no regions are sent to kbhost.
#ifndef DUI_IM_DISABLE_TRANSLUCENCY
    // Ditto for correction candidate widget
    subject->correctionCandidateWidget->showWidget();
    ++c1;
    QCOMPARE(spy.count(), c1);
    QCOMPARE(spy2.count(), c2);

    subject->correctionCandidateWidget->hide();
    ++c1;
    QCOMPARE(spy.count(), c1);
    QCOMPARE(spy2.count(), c2);
    QCOMPARE(region(spy, c1 - 1), region(spy2, c2 - 1));
#endif

    // But symbol view also changes input method area
    const int c1BeforeSymOpen = c1;
    subject->showSymbolView();
    waitForSignal(subject, SIGNAL(regionUpdated(const QRegion &)));
    ++c1;
    ++c2;
    QCOMPARE(spy.count(), c1);
    QCOMPARE(spy2.count(), c2);
    subject->symbolView->hideSymbolView();
    waitForSignal(subject, SIGNAL(regionUpdated(const QRegion &)));
    ++c1;
    ++c2;
    QCOMPARE(spy.count(), c1);
    QCOMPARE(spy2.count(), c2);
    // the same as before opening it
    QCOMPARE(region(spy, c1BeforeSymOpen - 1), region(spy, c1 - 1));

    // Hide the keyboard -> empty region and input method area
    subject->hide();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50); // really hidden after animation is finished
    ++c1;
    ++c2;
    QCOMPARE(spy.count(), c1);
    QCOMPARE(spy2.count(), c2);
    QCOMPARE(region(spy, c1 - 1), QRegion());
    QCOMPARE(region(spy2, c2 - 1), QRegion());

    // Regions and rotation

    // Preparation: store 270deg-angle region obtained as safely as possible
    spy.clear();
    spy2.clear();
    rotateToAngle(M::Angle270);
    subject->show();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);

    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy2.count(), 2);
    QCOMPARE(region(spy, 1), region(spy2, 1));

    QRegion region270(region(spy, 1));
    subject->hide();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);
    rotateToAngle(M::Angle0);

    QSignalSpy orientationSpy(window, SIGNAL(orientationChangeFinished(M::Orientation)));

    subject->show();
    QTest::qWait(MVirtualKeyboard::ShowHideTime + 50);
    spy.clear();
    spy2.clear();

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
    QVERIFY(spy.count() > 0);
    QCOMPARE(spy.count(), spy2.count());
    qDebug() << "Region after animation is finished:" << subject->vkbWidget->region();
    // Now, is the region sane after those consequtive rotations?
    QCOMPARE(region(spy, spy.count() - 1), region(spy2, spy2.count() - 1));
    // Remove the next two lines when QCOMPARE is enabled
    qDebug() << "Actual region:" << region(spy, spy.count() - 1);
    qDebug() << "Expected region:" << region270;
    // This fails at the moment. libmeegotouch/Qt bug suspected.
    //QCOMPARE(region(spy, spy.count() - 1), region270);
}

void Ut_MKeyboardHost::testSetState_data()
{
    QSet<MIMHandlerState> state;

    QTest::addColumn<QSet<MIMHandlerState> >("state");
    QTest::addColumn<int>("expectedCallCount");
    QTest::addColumn<MIMHandlerState>("expectedParameter");

    QTest::newRow("Empty") << state << 0 << OnScreen;

    state.clear();
    state << Hardware;
    QTest::newRow("Hardware") << state << 1 << Hardware;

    state.clear();
    state << OnScreen;
    QTest::newRow("OnScreen") << state << 0 << OnScreen;

    state.clear();
    state << Accessory;
    QTest::newRow("Accessory") << state << 1 << Accessory;

    state.clear();
    state << OnScreen << Hardware;
    QTest::newRow("Sequence1") << state << 0 << Hardware;

    state.clear();
    state << Hardware << Accessory;
    QTest::newRow("Sequence2") << state << 1 << Hardware;
}

void Ut_MKeyboardHost::testSetState()
{
    QFETCH(QSet<MIMHandlerState>, state);
    QFETCH(int, expectedCallCount);
    QFETCH(MIMHandlerState, expectedParameter);

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
    QSet<MIMHandlerState> state;

    gSetKeyboardStateCallCount = 0;
    state << Hardware;
    subject->update();
    subject->setState(state);
    QCOMPARE(gSetKeyboardStateCallCount, 1);
    QCOMPARE(gSetKeyboardStateParam, Hardware);
    subject->setState(state);
    QCOMPARE(gSetKeyboardStateCallCount, 1);
    QCOMPARE(gSetKeyboardStateParam, Hardware);

    state.clear();
    state << Hardware << OnScreen;
    subject->setState(state);
    QCOMPARE(gSetKeyboardStateCallCount, 2);
    QCOMPARE(gSetKeyboardStateParam, OnScreen);

    state.clear();
    state << OnScreen;
    subject->setState(state);
    QCOMPARE(gSetKeyboardStateCallCount, 2);
    QCOMPARE(gSetKeyboardStateParam, OnScreen);
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
    QSet<MIMHandlerState> state;

    //hardware state
    QVERIFY(subject->hardwareKeyboard);
    QSignalSpy spy(subject->hardwareKeyboard, SIGNAL(shiftStateChanged()));

    state << Hardware;
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
    state << OnScreen;
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

    //this value must be greater that MultitapTime in the file mkeyboardhost.cpp
    const int MultitapTime = 2000;

    subject->update();
    inputContext->preedit = "";
    inputContext->commit = "";

    if (!preedit.isEmpty()) {
        subject->preedit = preedit;
        subject->correctedPreedit = preedit;
    }

    for (int n = 0; n < event1.text().length(); ++n) {
        subject->handleKeyClick(event1);
        QCOMPARE(inputContext->preedit, preedit + QString(text[n]));
        QCOMPARE(inputContext->commit, QString(""));
    }
    QCOMPARE(inputContext->preedit, preedit + QString(text[text.length() - 1]));
    QCOMPARE(inputContext->commit, QString(""));

    subject->handleKeyClick(event2);
    QCOMPARE(inputContext->commit, preedit + QString(text[text.length() - 1]));
    QCOMPARE(inputContext->preedit, QString(event2.text()[0]));

    inputContext->commit = "";

    QTest::qWait(MultitapTime);
    subject->handleKeyClick(event2);
    QCOMPARE(inputContext->preedit, QString(event2.text()[0]));
    QCOMPARE(inputContext->commit, QString(event2.text()[0]));

    inputContext->commit  = "";
    inputContext->preedit = "";

    subject->handleKeyClick(space);
    QCOMPARE(inputContext->preedit, QString(""));
    QCOMPARE(inputContext->commit, QString(event2.text()[0]) + " ");
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
    QSet<MIMHandlerState> state;
    state << Hardware;
    subject->setState(state);
    QCOMPARE(inputContext->commit, text);
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
    QTest::addColumn<MIMHandlerState>("state");
    QTest::addColumn<ModifierState>("initialShiftState");
    QTest::addColumn<QString>("surroundingString");
    QTest::addColumn<bool>("autoCapitalizationEnabled");
    QTest::addColumn<int>("cursorPosition");
    QTest::addColumn<ModifierState>("expectedShiftState");

    // corsor is at the begining.
    QTest::newRow("screen lowercase") << OnScreen << ModifierClearState << QString("Test. ")
                                      << true << 0 << ModifierLatchedState;
    QTest::newRow("screen latched") << OnScreen << ModifierLatchedState << QString("Test. ")
                                    << true << 2 << ModifierClearState;
    // cursor is right after a dot.
    QTest::newRow("screen latched") << OnScreen << ModifierLatchedState << QString("Test. ")
                                    << true << 6 << ModifierLatchedState;
    QTest::newRow("screen latched") << OnScreen << ModifierLatchedState << QString("Test. ")
                                    << true << 2  << ModifierClearState;
    QTest::newRow("screen locked") << OnScreen << ModifierLockedState << QString("Test. ")
                                   << true << 0 << ModifierLockedState;
    QTest::newRow("screen locked") << OnScreen << ModifierLockedState << QString("Test. ")
                                   << true << 2 << ModifierLockedState;
    // text entry disable autocaps.
    QTest::newRow("screen locked") << OnScreen << ModifierLatchedState << QString("Test. ")
                                   << false << 0 << ModifierClearState;
}

void Ut_MKeyboardHost::testShiftStateOnFocusChanged()
{
    // all temporary shift state (not capslock) should be reset when
    // focus is changed, and new shift state depends on autocaps
    QFETCH(MIMHandlerState, state);
    QFETCH(ModifierState, initialShiftState);
    QFETCH(QString, surroundingString);
    QFETCH(bool, autoCapitalizationEnabled);
    QFETCH(int, cursorPosition);
    QFETCH(ModifierState, expectedShiftState);

    QSet<MIMHandlerState> set;
    set << state;
    subject->setState(set);

    subject->vkbWidget->setShiftState(initialShiftState);

    subject->focusChanged(true);
    inputContext->surroundingString = surroundingString;
    inputContext->autoCapitalizationEnabled_ = autoCapitalizationEnabled;
    inputContext->cursorPos = cursorPosition;
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

    QSet<MIMHandlerState> set;
    set << OnScreen;
    subject->setState(set);

    inputContext->surroundingString = surroundingString;
    inputContext->autoCapitalizationEnabled_ = autoCapitalizationEnabled;
    inputContext->cursorPos = cursorPosition;
    subject->vkbWidget->setShiftState(initialShiftState);

    gAutoCapsEnabled = layoutAutoCapitalization;
    subject->handleVirtualKeyboardLayoutChanged(layout);

    QCOMPARE(subject->vkbWidget->shiftStatus(), expectedShiftState);
}

void Ut_MKeyboardHost::rotateToAngle(M::OrientationAngle angle)
{
    subject->appOrientationChanged(angle);
    QTest::qWait(SceneRotationTime); // wait until rotation animation is finished
}

void Ut_MKeyboardHost::triggerAutoCaps()
{
    // Set necessary conditions for triggering auto capitalization
    gAutoCapsEnabled = true;
    subject->preedit.clear();
    subject->cursorPos = 0;
    inputContext->contentType_ = M::FreeTextContentType;
    inputContext->autoCapitalizationEnabled_ = true;

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

    ok = toolbar1->loadNokiaToolbarXml(toolbarName1);
    QVERIFY2(ok, "toolbar1.xml was not loaded correctly");

    ok = toolbar2->loadNokiaToolbarXml(toolbarName2);
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
    QTest::addColumn<MInputMethodBase::InputModeIndicator>("expectIndicator");
    QTest::addColumn<bool>("notificationShowCalled");

    QTest::newRow("English clear indicator") << QString("us") << QString("") << 0
                                             << 0 << MInputMethodBase::LatinLower
                                             << false;
    QTest::newRow("English shift latched indicator") << QString("us") << QString("") << 1
                                                     << 0 << MInputMethodBase::LatinUpper
                                                     << false;
    QTest::newRow("English shift locked indicator") << QString("us") << QString("") << 2
                                                    << 0 << MInputMethodBase::LatinLocked
                                                    << true;
    QTest::newRow("English fn latched indicator") << QString("us") << QString("") << 0
                                                  << 1 << MInputMethodBase::NumAndSymLatched
                                                  << false;
    QTest::newRow("English fn locked indicator") << QString("us") << QString("") << 0
                                                 << 2 << MInputMethodBase::NumAndSymLocked
                                                 << true;
    QTest::newRow("Cyrillic clear indicator") << QString("ru") << QString("cyrillic") << 0
                                              << 0 << MInputMethodBase::CyrillicLower
                                              << false;
    QTest::newRow("Cyrillic shift latched indicator") << QString("ru") << QString("cyrillic") << 1
                                                      << 0 << MInputMethodBase::CyrillicUpper
                                                      << false;
    QTest::newRow("Cyrillic shift locked indicator") << QString("ru") << QString("cyrillic") << 2
                                                     << 0 << MInputMethodBase::CyrillicLocked
                                                     << true;
    QTest::newRow("Latin clear indicator") << QString("ru") << QString("latin") << 0
                                              << 0 << MInputMethodBase::LatinLower
                                              << false;
    QTest::newRow("Latin shift latched indicator") << QString("ru") << QString("latin") << 1
                                                   << 0 << MInputMethodBase::LatinUpper
                                                   << false;
    QTest::newRow("Latin shift locked indicator") << QString("us") << QString("latin") << 2
                                                  << 0 << MInputMethodBase::LatinLocked
                                                  << true;
    QTest::newRow("Arabic indicator") << QString("ara") << QString("") << 0
                                      << 0 << MInputMethodBase::Arabic
                                      << false;
}

void Ut_MKeyboardHost::testHandleHwKeyboardStateChanged()
{
    QFETCH(QString, xkbLayout);
    QFETCH(QString, xkbVariant);
    QFETCH(int, shiftClickedCount);
    QFETCH(int, fnClickedCount);
    QFETCH(MInputMethodBase::InputModeIndicator, expectIndicator);
    QFETCH(bool, notificationShowCalled);

    LayoutsManager::instance().setXkbMap(xkbLayout, xkbVariant);
    QSet<MIMHandlerState> states;
    states << Hardware;
    subject->setState(states);
    subject->focusChanged(true);
    gShowLockOnInfoBannerCallCount = 0;

    for (int i = 0; i < shiftClickedCount;  i++) {
        subject->processKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, QString(""), false, 1, 0, 0);
        subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, Qt::ShiftModifier, QString(""), false, 1, 0, 0);
    }

    for (int i = 0; i < fnClickedCount;  i++) {
        subject->processKeyEvent(QEvent::KeyPress, FnLevelKey, Qt::NoModifier, QString(""), false, 1, 0, 0);
        subject->processKeyEvent(QEvent::KeyRelease, FnLevelKey, FnLevelModifier, QString(""), false, 1, 0, 0);
    }

    QCOMPARE(gInputMethodIndicator, expectIndicator);
    QCOMPARE(gShowLockOnInfoBannerCallCount, notificationShowCalled ? 1 : 0);
}

QTEST_APPLESS_MAIN(Ut_MKeyboardHost);

