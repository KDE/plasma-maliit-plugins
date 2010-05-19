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
#include <layoutmenu.h>
#include <symbolview.h>

#include "duiimenginewords.h"

#include "mgconfitem_stub.h"
#include "minputcontextstubconnection.h"
#include "ut_mkeyboardhost.h"

#include <MApplication>
#include <MSceneManager>
#include <MTheme>
#include "mplainwindow.h"
#include <mnamespace.h>
#include <MWidgetController>
#include <MDialog>

#include <QDir>

#include <X11/X.h>
#undef KeyPress
#undef KeyRelease

namespace
{
    const QString InputMethodCorrectionSetting("/meegotouch/inputmethods/correctionenabled");
    const QString InputMethodCorrectionEngine("/meegotouch/inputmethods/correctionengine");
    bool gAccurateMode = false;
    int gSetKeyboardStateCallCount = 0;
    MIMHandlerState gSetKeyboardStateParam = OnScreen;
    const int LayoutMenuShowTime = 300; // in ms
    const int SceneRotationTime = 1400; // in ms

    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";
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

bool MVirtualKeyboard::isAccurateMode() const
{
    return gAccurateMode;
}

QString MVirtualKeyboard::layoutLanguage() const
{
    return QString("fi");
}


// Actual test...............................................................

void Ut_MKeyboardHost::initTestCase()
{
    static int argc = 1;
    static char *app_name[1] = { (char *)"ut_mvirtualkeyboardhost" };

    // Avoid waiting if im server is not responding
    MApplication::setLoadMInputContext(false);
    app = new MApplication(argc, app_name);
    MTheme::instance()->loadCSS("/usr/share/meegotouch/virtual-keyboard/css/864x480.css");
    inputContext = new MInputContextStubConnection;
    window = new MPlainWindow;

    MGConfItem(MultitouchSettings).set(true);
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

    subject = new MKeyboardHost(inputContext, 0);
    inputContext->clear();

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

    gAccurateMode = false;
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

    gAccurateMode = false;
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

    subject->handleKeyClick(KeyEvent("m"));
    gAccurateMode = true;
    subject->handleKeyClick(KeyEvent("a"));
    QCOMPARE(inputContext->commit, QString("ma"));
    QVERIFY(subject->preedit.isEmpty());

    gAccurateMode = false;
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
    inputContext->surrodingString = "Test string. You can using it!    ";
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
    QCOMPARE(region(spy, c1 - 1), region(spy2, 1));

    // When layout menu is shown, input method area doesn't change...
    QTimer::singleShot(LayoutMenuShowTime, subject->layoutMenu->keyboardOptionDialog, SLOT(reject()));
    qDebug() << "Opening and closing layout menu...";
    subject->showLayoutMenu();
    waitForSignal(subject->layoutMenu, SIGNAL(regionUpdated(const QRegion&)));
    qDebug() << "...layout menu closed.";
    c1 += 2;
    QCOMPARE(spy.count(), c1);
    QCOMPARE(spy2.count(), c2);
    // ...and after closing the region is again the same as the input method area
    QCOMPARE(region(spy, c1 - 1), region(spy2, c2 - 1));

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
    QCOMPARE(spy2.count(), 2);
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
    QCOMPARE(region(spy, c1BeforeSymOpen - 1), region(spy, c1 - 1)); // the same as before opening it

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

void Ut_MKeyboardHost::rotateToAngle(M::OrientationAngle angle)
{
    subject->appOrientationChanged(angle);
    QTest::qWait(SceneRotationTime); // wait until rotation animation is finished
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
    QVERIFY(!subject->symbolView->isActive());
    //symbol key click will active symbol view and switch the pages
    int currentPageIndex;
    for (int i = 0; i < subject->symbolView->pageCount(); i++) {
        subject->handleSymbolKeyClick();
        QVERIFY(subject->symbolView->isActive());
        currentPageIndex = subject->symbolView->currentPage();
        QCOMPARE(currentPageIndex, i);
    }
    //if reach the last page, then symbol view will be closed
    subject->handleSymbolKeyClick();
    QVERIFY(!subject->symbolView->isActive());
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
    QSignalSpy spy1(subject->vkbWidget, SIGNAL(shiftLevelChanged()));

    subject->autoCapsEnabled = false; // disable auto caps
    subject->vkbWidget->setShiftState(ModifierClearState);
    state.clear();
    state << OnScreen;
    subject->setState(state);
    QVERIFY(!subject->symbolView->isActive());
    subject->symbolView->showSymbolView();
    QVERIFY(subject->symbolView->isActive());
    QCOMPARE(subject->symbolView->currentLevel(), 0);
    QCOMPARE(subject->vkbWidget->shiftStatus(), ModifierClearState);

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
    QTest::addColumn<bool>("accurateMode");
    QTest::addColumn<QString>("preedit");

    QTest::newRow("fast mode") << false << "";
    QTest::newRow("fast mode, preedit") << false << "preedit";
    QTest::newRow("accurate mode") << true << "";
}

void Ut_MKeyboardHost::testKeyCycle()
{
    QFETCH(bool, accurateMode);
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

    gAccurateMode = accurateMode;
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

void Ut_MKeyboardHost::testPressShift_data()
{
    QTest::addColumn<MIMHandlerState>("state");
    QTest::addColumn<ModifierState>("initialShiftState");
    QTest::addColumn<ModifierState>("expectedShiftState");
    QTest::addColumn<bool>("enableMultiTouch");

    QTest::newRow("screen lowercase") << OnScreen << ModifierClearState   << ModifierLatchedState << true;
    QTest::newRow("screen latched")   << OnScreen << ModifierLatchedState << ModifierClearState << true;
    QTest::newRow("screen locked")    << OnScreen << ModifierLockedState  << ModifierClearState << true;

    QTest::newRow("hw lowercase") << Hardware << ModifierClearState   << ModifierClearState << true;
    QTest::newRow("hw latched")   << Hardware << ModifierLatchedState << ModifierLatchedState << true;
    QTest::newRow("hw locked")    << Hardware << ModifierLockedState  << ModifierLockedState << true;

    QTest::newRow("single-touch lowercase") << OnScreen << ModifierClearState   << ModifierClearState << false;
    QTest::newRow("single-touch latched")   << OnScreen << ModifierLatchedState << ModifierLatchedState << false;
    QTest::newRow("single-touch locked")    << OnScreen << ModifierLockedState  << ModifierLockedState << false;
}

void Ut_MKeyboardHost::testPressShift()
{
    QFETCH(MIMHandlerState, state);
    QFETCH(ModifierState, initialShiftState);
    QFETCH(ModifierState, expectedShiftState);
    QFETCH(bool, enableMultiTouch);

    QSet<MIMHandlerState> set;
    set << state;

    KeyEvent pressShift(QString(), QEvent::KeyPress, Qt::Key_Shift, KeyEvent::NotSpecial, Qt::ShiftModifier);

    subject->setState(set);
    subject->vkbWidget->setShiftState(initialShiftState);
    subject->enableMultiTouch = enableMultiTouch;

    subject->handleKeyPress(pressShift);
    QVERIFY(subject->vkbWidget->shiftStatus() == expectedShiftState);

    subject->handleKeyPress(pressShift);
    QVERIFY(subject->vkbWidget->shiftStatus() == expectedShiftState);
}

void Ut_MKeyboardHost::testReleaseShift_data()
{
    QTest::addColumn<MIMHandlerState>("state");
    QTest::addColumn<ModifierState>("initialShiftState");
    QTest::addColumn<ModifierState>("expectedShiftState");
    QTest::addColumn<bool>("upperCase");
    QTest::addColumn<bool>("upperCase2");

    QTest::newRow("screen lowercase") << OnScreen << ModifierClearState   << ModifierClearState   << false << false;
    QTest::newRow("screen latched")   << OnScreen << ModifierLatchedState << ModifierLatchedState << false << false;
    QTest::newRow("screen locked")    << OnScreen << ModifierLockedState  << ModifierLockedState  << false << false;

    QTest::newRow("screen lowercase 2") << OnScreen << ModifierClearState   << ModifierLatchedState << false << true;
    QTest::newRow("screen latched   2") << OnScreen << ModifierLatchedState << ModifierClearState   << true  << false;
    QTest::newRow("screen locked    2") << OnScreen << ModifierLockedState  << ModifierLatchedState << false << true;

    QTest::newRow("hw lowercase") << Hardware << ModifierClearState   << ModifierClearState   << false << false;
    QTest::newRow("hw latched")   << Hardware << ModifierLatchedState << ModifierLatchedState << false << false;
    QTest::newRow("hw locked")    << Hardware << ModifierLockedState  << ModifierLockedState  << false << false;
}

void Ut_MKeyboardHost::testReleaseShift()
{
    QFETCH(MIMHandlerState, state);
    QFETCH(ModifierState, initialShiftState);
    QFETCH(ModifierState, expectedShiftState);
    QFETCH(bool, upperCase);
    QFETCH(bool, upperCase2);

    QSet<MIMHandlerState> set;
    set << state;

    KeyEvent pressShift  (QString(), QEvent::KeyPress,   Qt::Key_Shift, KeyEvent::NotSpecial, Qt::ShiftModifier);
    KeyEvent releaseShift(QString(), QEvent::KeyRelease, Qt::Key_Shift, KeyEvent::NotSpecial, Qt::ShiftModifier);

    subject->setState(set);
    subject->vkbWidget->setShiftState(initialShiftState);
    subject->upperCase = upperCase;

    subject->handleKeyPress(pressShift);
    subject->upperCase = upperCase2;
    subject->handleKeyRelease(releaseShift);
    QVERIFY(subject->vkbWidget->shiftStatus() == expectedShiftState);

    subject->handleKeyRelease(releaseShift);
    QVERIFY(subject->vkbWidget->shiftStatus() == expectedShiftState);
}

QTEST_APPLESS_MAIN(Ut_MKeyboardHost);

