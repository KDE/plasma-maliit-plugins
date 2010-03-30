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



#include <duiimcorrectioncandidatewidget.h>
#include <duivirtualkeyboard.h>
#include <duihardwarekeyboard.h>
#include <duikeyboardhost.h>
#include <duivirtualkeyboardstyle.h>
#include <layoutmenu.h>
#include <symbolview.h>

#include "duigconfitem_stub.h"
#include "duiimenginewords.h"
#include "duiinputcontextstubconnection.h"
#include "ut_duikeyboardhost.h"

#include <DuiApplication>
#include <DuiSceneManager>
#include <DuiTheme>
#include "duiplainwindow.h"
#include <duinamespace.h>
#include <DuiWidgetController>
#include <DuiDialog>

#include <QDir>

namespace
{
    const QString InputMethodCorrectionSetting("/Dui/InputMethods/CorrectionEnabled");
    const QString InputMethodCorrectionEngine("/Dui/InputMethods/CorrectionEngine");
    bool gAccurateMode = false;
    int gSetKeyboardStateCallCount = 0;
    DuiIMHandlerState gSetKeyboardStateParam = OnScreen;
    const int LayoutMenuShowTime = 300; // in ms
    const int SceneRotationTime = 1400; // in ms
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

Q_DECLARE_METATYPE(QList<DuiIMHandlerState>)
Q_DECLARE_METATYPE(DuiIMHandlerState)


static void waitForSignal(const QObject* object, const char* signal, int timeout = 500)
{
    QEventLoop eventLoop;
    QObject::connect(object, signal, &eventLoop, SLOT(quit()));
    QTimer::singleShot(timeout, &eventLoop, SLOT(quit()));
    eventLoop.exec();
}


// Stubbing..................................................................

void DuiVirtualKeyboard::setKeyboardState(DuiIMHandlerState state)
{
    ++gSetKeyboardStateCallCount;
    gSetKeyboardStateParam = state;
}

bool DuiVirtualKeyboard::isAccurateMode() const
{
    return gAccurateMode;
}

QString DuiVirtualKeyboard::layoutLanguage() const
{
    return QString("fi");
}


// Actual test...............................................................

void Ut_DuiKeyboardHost::initTestCase()
{
    static int argc = 1;
    static char *app_name[1] = { (char *)"ut_duivirtualkeyboardhost" };

    // Avoid waiting if im server is not responding
    DuiApplication::setLoadDuiInputContext(false);
    app = new DuiApplication(argc, app_name);
    DuiTheme::instance()->loadCSS("/usr/share/dui/virtual-keyboard/css/864x480.css");
    inputContext = new DuiInputContextStubConnection;
    window = new DuiPlainWindow;
    if (DuiPlainWindow::instance()->orientationAngle() != Dui::Angle0) {
        DuiPlainWindow::instance()->setOrientationAngle(Dui::Angle0);
        QTest::qWait(1000);
    }
}

void Ut_DuiKeyboardHost::cleanupTestCase()
{
    delete window;
    window = 0;
    delete inputContext;
    inputContext = 0;
    delete app;
    app = 0;
}

void Ut_DuiKeyboardHost::init()
{
    // Uses dummy driver
    DuiGConfItem engineConfig(InputMethodCorrectionEngine);
    engineConfig.set(QVariant(QString("dummyimdriver")));

    subject = new DuiKeyboardHost(inputContext, 0);
    inputContext->clear();

    if (DuiPlainWindow::instance()->orientationAngle() != Dui::Angle0) {
        DuiPlainWindow::instance()->setOrientationAngle(Dui::Angle0);
        QTest::qWait(1000);
    }
}

void Ut_DuiKeyboardHost::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_DuiKeyboardHost::testCreate()
{
    QVERIFY(subject != 0);
}

void Ut_DuiKeyboardHost::testRotatePoint()
{
    QPoint position(100, 200);
    QPoint result;
    int displayWidth = DuiPlainWindow::instance()->visibleSceneSize().width();
    int displayHeight = DuiPlainWindow::instance()->visibleSceneSize().height();
    bool isOk = false;

    rotateToAngle(Dui::Angle0);

    isOk = subject->rotatePoint(position, result);
    QVERIFY(isOk == true);
    QCOMPARE(result, position);

    rotateToAngle(Dui::Angle90);

    isOk = subject->rotatePoint(position, result);
    QVERIFY(isOk == true);
    QCOMPARE(result, QPoint(200, displayWidth - 100));

    rotateToAngle(Dui::Angle270);

    isOk = subject->rotatePoint(position, result);
    QVERIFY(isOk == true);
    QCOMPARE(result, QPoint(displayHeight - 200, 100));

    rotateToAngle(Dui::Angle180);

    isOk = subject->rotatePoint(position, result);
    QVERIFY(isOk == true);
    QCOMPARE(result, QPoint(displayWidth - 100, displayHeight - 200));
}


void Ut_DuiKeyboardHost::testRotateRect()
{
    QRect rect;
    QRect result(1, 2, 3, 4);
    int displayWidth = DuiPlainWindow::instance()->visibleSceneSize().width();
    int displayHeight = DuiPlainWindow::instance()->visibleSceneSize().height();

    QList<QRect> rects;
    QList<Dui::OrientationAngle> angles;
    QList<QRect> expected;

    // invalid rectangles
    rects.append(QRect(100, 200, -20, 40));
    angles.append(Dui::Angle0);
    expected.append(QRect());

    rects.append(QRect(0, 0, -1, -1));
    angles.append(Dui::Angle90);
    expected.append(QRect());

    rects.append(QRect(1, 1, 1, -1));
    angles.append(Dui::Angle180);
    expected.append(QRect());

    // invalid angle
    rects.append(QRect(1, 1, 1, 1));
    angles.append((Dui::OrientationAngle)(-1));
    expected.append(QRect());

    // valid rectangles
    rect = QRect(100, 200, 20, 40);
    rects.append(rect);
    angles.append(Dui::Angle0);
    expected.append(rect);

    rect = QRect(100, 200, 20, 40);
    rects.append(rect);
    angles.append(Dui::Angle90);
    expected.append(QRect(rect.y(), displayWidth - rect.x() - rect.width(), rect.height(), rect.width()));

    rect = QRect(-4, 4, 20, 40);
    rects.append(rect);
    angles.append(Dui::Angle180);
    expected.append(QRect(
                        displayWidth - rect.x() - rect.width(),
                        displayHeight - rect.y() - rect.height(),
                        rect.width(), rect.height()));

    rect = QRect(10, 200, 20, 40);
    rects.append(rect);
    angles.append(Dui::Angle270);
    expected.append(QRect(
                        displayHeight - rect.y() - rect.height(), rect.x(),
                        rect.height(), rect.width()));

    for (int i = 0; i < rects.length(); ++i) {
        rect = rects.at(i);
        Dui::OrientationAngle angle = angles.at(i);
        rotateToAngle(angle);

        bool validAngle = (
                              angle == Dui::Angle0   ||
                              angle == Dui::Angle90  ||
                              angle == Dui::Angle180 ||
                              angle == Dui::Angle270);
        bool rotated = subject->rotateRect(rect, result);
        QCOMPARE(rotated, rect.isValid() && validAngle);
        QCOMPARE(result, expected.at(i));
    }
}


void Ut_DuiKeyboardHost::testHandleClick()
{
    DuiGConfItem config(InputMethodCorrectionSetting);
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

void Ut_DuiKeyboardHost::testDirectMode()
{
    QList<KeyEvent> testData;
    QList<Qt::Key> expectedKeys;

    testData << KeyEvent("\b", QEvent::KeyRelease, Qt::Key_Backspace)
             << KeyEvent("\n", QEvent::KeyRelease, Qt::Key_Return)
             << KeyEvent(" ", QEvent::KeyRelease, Qt::Key_Space);
    expectedKeys << Qt::Key_Backspace << Qt::Key_Return << Qt::Key_Space;
    QVERIFY(testData.count() == expectedKeys.count());

    subject->inputMethodMode = Dui::InputMethodModeDirect;

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

void Ut_DuiKeyboardHost::testNotCrash()
{
    //at least we should not crash
    subject->show();
    subject->hide();
    subject->reset();
    subject->setPreedit("string");
    subject->initializeInputEngine();
    subject->mouseClickedOnPreedit(QPoint(0, 0), QRect(0, 0, 1, 1));
}

void Ut_DuiKeyboardHost::testErrorCorrectionOption()
{
    subject->show();
    DuiGConfItem config(InputMethodCorrectionSetting);

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

void Ut_DuiKeyboardHost::testAutoCaps()
{
    inputContext->surrodingString = "Test string. You can using it!    ";
    inputContext->autoCapitalizationEnabled_ = true;
    subject->correctionEnabled = true;
    inputContext->contentType_ = Dui::FreeTextContentType;
    subject->show();

    inputContext->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOn);

    inputContext->cursorPos = 1;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOff);

    inputContext->cursorPos = 12;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOff);

    inputContext->cursorPos = 13;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOn);


    subject->hide();
    subject->show();

    inputContext->cursorPos = 13;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOn);

    inputContext->cursorPos = 16;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOff);

    inputContext->cursorPos = 31;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOn);

    inputContext->cursorPos = 33;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOff);

    inputContext->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOn);
    // When autoCaps is on and shift is latched, any key input except shift and backspace (in an sepcial case)
    // will turn off shift.
    KeyEvent press("a", QEvent::KeyPress);
    KeyEvent release(press, QEvent::KeyRelease);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOff);

    inputContext->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOn);
    // The special case for backspace when autoCaps is on, that is cursor is at 0 position,
    // should not change the shift state.
    press = KeyEvent("", QEvent::KeyPress, Qt::Key_Backspace);
    release = KeyEvent(press, QEvent::KeyRelease);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOn);

    // If cursor is not at 0 position, backspace should also change the shift state.
    inputContext->cursorPos = 2;
    subject->update();
    subject->vkbWidget->setShiftState(DuiVirtualKeyboard::ShiftOn);
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOn);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOff);

    // Test holding backspace with preedit.
    press = KeyEvent("", QEvent::KeyPress, Qt::Key_Backspace);
    release = KeyEvent(press, QEvent::KeyRelease);
    inputContext->cursorPos = 18;
    subject->preedit = "You can use";
    // initial state: preedit("You can use"), shift state:latched, start holding backspace
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOff);
    subject->vkbWidget->setShiftState(DuiVirtualKeyboard::ShiftOn);
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOn);

    // press and release backspace before timeout will only delete one character,
    subject->handleKeyPress(press);
    QVERIFY(subject->backSpaceTimer.isActive());
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOff);
    QCOMPARE(subject->preedit, QString("You can us"));

    // but hold backspace longer than timeout, will delete the whole preedit.
    subject->handleKeyPress(press);
    int interval = subject->backSpaceTimer.interval();
    QTest::qWait(interval / 2);
    QVERIFY(subject->backSpaceTimer.isActive());
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOff);
    QTest::qWait((interval / 2) + 50);
    // final state: preedit(""), shift state:on, after holding backspace enough time.
    QVERIFY(subject->preedit.isEmpty());
    QVERIFY(!subject->backSpaceTimer.isActive());
    inputContext->cursorPos = 13;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOn);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOn);

    subject->hide();
    QTest::qWait(DuiVirtualKeyboard::ShowHideTime + 50);

    // Disable autoCaps
    inputContext->autoCapitalizationEnabled_ = false;
    inputContext->cursorPos = 0;
    subject->show();
    subject->update();
    QTest::qWait(DuiVirtualKeyboard::ShowHideTime + 50);
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOff);

    // When shift is latched, any key input except shift will turn off shift.
    subject->vkbWidget->setShiftState(DuiVirtualKeyboard::ShiftOn);
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOn);
    press = KeyEvent("a", QEvent::KeyPress);
    release = KeyEvent(press, QEvent::KeyRelease);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOff);

    // Backspace will also change the shift state when cursor is at 0 position.
    inputContext->cursorPos = 0;
    subject->vkbWidget->setShiftState(DuiVirtualKeyboard::ShiftOn);
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOn);
    press = KeyEvent("", QEvent::KeyPress, Qt::Key_Backspace);
    release = KeyEvent(press, QEvent::KeyRelease);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == DuiVirtualKeyboard::ShiftOff);
}

void Ut_DuiKeyboardHost::testApplicationOrientationChanged()
{
    DuiInputMethodBase *im = subject;
    Dui::OrientationAngle angles[] = { Dui::Angle0, Dui::Angle90, Dui::Angle180, Dui::Angle270 };

    for (int i = 0; i < 5; ++i) {
        Dui::OrientationAngle currentAngle = angles[i % 4];
        im->appOrientationChanged(static_cast<int>(currentAngle));
        QTest::qWait(1500);
        QCOMPARE(currentAngle, DuiPlainWindow::instance()->orientationAngle());
    }
}

void Ut_DuiKeyboardHost::testCopyPaste()
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

void Ut_DuiKeyboardHost::testPlusMinus()
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

void Ut_DuiKeyboardHost::testRegionSignals()
{
    qRegisterMetaType<QRegion>("QRegion");
    QSignalSpy spy(subject, SIGNAL(regionUpdated(QRegion)));
    QSignalSpy spy2(subject, SIGNAL(inputMethodAreaUpdated(QRegion)));

    subject->show();
    // We must immediately get non-empty region so that passthrough window
    // is made visible right before the animation starts.
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy2.count(), 1);
    QCOMPARE(region(spy, 0), region(spy2, 0));
    QVERIFY(!region(spy, 0).isEmpty());

    // We must get another, larger region when the vkb is fully visible
    QTest::qWait(DuiVirtualKeyboard::ShowHideTime + 50);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy2.count(), 2);
    qDebug() << "Passthrough region: " << region(spy, 1);
    qDebug() << "libdui region: " << region(spy2, 1);
    QCOMPARE(region(spy, 1), region(spy2, 1));
    QVERIFY(!(region(spy, 1) - region(spy, 0)).isEmpty());

    // When layout menu is shown, input method area doesn't change...
    QTimer::singleShot(LayoutMenuShowTime, subject->layoutMenu->keyboardOptionDialog, SLOT(reject()));
    qDebug() << "Opening and closing layout menu...";
    subject->showLayoutMenu();
    waitForSignal(subject->layoutMenu, SIGNAL(regionUpdated(const QRegion&)));
    qDebug() << "...layout menu closed.";
    QCOMPARE(spy.count(), 4);
    QCOMPARE(spy2.count(), 2);
    // ...and after closing the region is again the same as the input method area
    QCOMPARE(region(spy, 3), region(spy2, 1));

    // Ditto for correction candidate widget
    subject->correctionCandidateWidget->showWidget();
    QCOMPARE(spy.count(), 5);
    QCOMPARE(spy2.count(), 2);

    subject->correctionCandidateWidget->hide();
    QCOMPARE(spy.count(), 6);
    QCOMPARE(spy2.count(), 2);
    QCOMPARE(region(spy, 5), region(spy2, 1));

    // But symbol view also changes input method area
    subject->showSymbolView();
    waitForSignal(subject, SIGNAL(regionUpdated(const QRegion &)));
    QCOMPARE(spy.count(), 7);
    QCOMPARE(spy2.count(), 3);
    subject->symbolView->hideSymbolView();
    waitForSignal(subject, SIGNAL(regionUpdated(const QRegion &)));
    QCOMPARE(spy.count(), 8);
    QCOMPARE(spy2.count(), 4);
    QCOMPARE(region(spy, 5), region(spy, 7)); // the same as before opening it

    // Hide the keyboard -> empty region and input method area
    subject->hide();
    QTest::qWait(DuiVirtualKeyboard::ShowHideTime + 50); // really hidden after animation is finished
    QCOMPARE(spy.count(), 9);
    QCOMPARE(spy2.count(), 5);
    QCOMPARE(region(spy, 8), QRegion());
    QCOMPARE(region(spy2, 4), QRegion());

    // Regions and rotation

    // Preparation: store 270deg-angle region obtained as safely as possible
    spy.clear();
    spy2.clear();
    rotateToAngle(Dui::Angle270);
    subject->show();
    QTest::qWait(DuiVirtualKeyboard::ShowHideTime + 50);
    QCOMPARE(region(spy, 1), region(spy2, 1));
    QRegion region270(region(spy, 1));
    subject->hide();
    QTest::qWait(DuiVirtualKeyboard::ShowHideTime + 50);
    rotateToAngle(Dui::Angle0);

    QSignalSpy orientationSpy(DuiPlainWindow::instance()->sceneManager(),
                              SIGNAL(orientationChangeFinished(Dui::Orientation)));

    subject->show();
    QTest::qWait(DuiVirtualKeyboard::ShowHideTime + 50);
    spy.clear();
    spy2.clear();

    // Rotate three times repeatedly with long and short waits in between.  We
    // should end up with a region identical to that stored in region270.  The
    // wait times mimic user operations that have been found to cause a problem.
    DuiPlainWindow::instance()->sceneManager()->setOrientationAngle(Dui::Angle90);
    QTest::qWait(800);
    qDebug() << "Orientations finished:" << orientationSpy.count();
    DuiPlainWindow::instance()->sceneManager()->setOrientationAngle(Dui::Angle180);
    QTest::qWait(5);
    DuiPlainWindow::instance()->sceneManager()->setOrientationAngle(Dui::Angle270);
    qDebug() << "Orientations finished:" << orientationSpy.count();
    qDebug() << "Waiting for rotation animation to finish...";
    QTest::qWait(SceneRotationTime); // wait until rotation animation is finished
    qDebug() << "Waiting for rotation animation to finish...done!";
    // Sanity checks
    qDebug() << "Orientations finished:" << orientationSpy.count();
    QCOMPARE(DuiPlainWindow::instance()->sceneManager()->orientationAngle(), Dui::Angle270);
    QVERIFY(spy.count() > 0);
    QCOMPARE(spy.count(), spy2.count());
    qDebug() << "Region after animation is finished:" << subject->vkbWidget->region();
    // Now, is the region sane after those consequtive rotations?
    QCOMPARE(region(spy, spy.count() - 1), region(spy2, spy2.count() - 1));
    // Remove the next two lines when QCOMPARE is enabled
    qDebug() << "Actual region:" << region(spy, spy.count() - 1);
    qDebug() << "Expected region:" << region270;
    // This fails at the moment. libdui/Qt bug suspected.
    //QCOMPARE(region(spy, spy.count() - 1), region270);
}

void Ut_DuiKeyboardHost::rotateToAngle(Dui::OrientationAngle angle)
{
    DuiPlainWindow::instance()->setOrientationAngle(angle);
    QTest::qWait(SceneRotationTime); // wait until rotation animation is finished
}

void Ut_DuiKeyboardHost::testSetState_data()
{
    QList<DuiIMHandlerState> state;

    QTest::addColumn<QList<DuiIMHandlerState> >("state");
    QTest::addColumn<int>("expectedCallCount");
    QTest::addColumn<DuiIMHandlerState>("expectedParameter");

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
    QTest::newRow("Sequence1") << state << 1 << Hardware;

    state.clear();
    state << Hardware << OnScreen;
    QTest::newRow("Sequence2") << state << 0 << OnScreen;
}

void Ut_DuiKeyboardHost::testSetState()
{
    QFETCH(QList<DuiIMHandlerState>, state);
    QFETCH(int, expectedCallCount);
    QFETCH(DuiIMHandlerState, expectedParameter);

    qDebug() << "Probe state=" << state;

    gSetKeyboardStateCallCount = 0;
    subject->update();
    subject->setState(state);
    QCOMPARE(gSetKeyboardStateCallCount, expectedCallCount);
    if (gSetKeyboardStateCallCount) {
        QCOMPARE(gSetKeyboardStateParam, expectedParameter);
    }
}

void Ut_DuiKeyboardHost::testSetStateCombination()
{
    QList<DuiIMHandlerState> state;

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

void Ut_DuiKeyboardHost::testSymbolKeyClick()
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

void Ut_DuiKeyboardHost::testUpdateSymbolViewLevel()
{
    subject->show();
    QList<DuiIMHandlerState> state;

    //hardware state
    QVERIFY(subject->hardwareKeyboard);
    QSignalSpy spy(subject->hardwareKeyboard, SIGNAL(shiftLevelChanged()));

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
    subject->processKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, QString(""), false, 1, 0);
    subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, Qt::ShiftModifier, QString(""), false, 1, 0);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(subject->hardwareKeyboard->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    QCOMPARE(subject->symbolView->currentLevel(), 1);
    //! second shift key press+release will lock the shift modifier, and still keep the symbolview level as 1
    subject->processKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, QString(""), false, 1, 0);
    subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, Qt::ShiftModifier, QString(""), false, 1, 0);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(subject->hardwareKeyboard->modifierState(Qt::ShiftModifier), ModifierLockedState);
    QCOMPARE(subject->symbolView->currentLevel(), 1);
    //! third shift key press+release will clear the shift modifier, and switch the symbolview level back to 0
    subject->processKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, QString(""), false, 1, 0);
    subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, Qt::ShiftModifier, QString(""), false, 1, 0);
    QCOMPARE(spy.count(), 3);
    QCOMPARE(subject->hardwareKeyboard->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(subject->symbolView->currentLevel(), 0);
    subject->symbolView->hideSymbolView();

    //onscreen state
    QVERIFY(subject->vkbWidget);
    QSignalSpy spy1(subject->vkbWidget, SIGNAL(shiftLevelChanged()));

    subject->autoCapsEnabled = false; // disable auto caps
    state.clear();
    state << OnScreen;
    subject->setState(state);
    QVERIFY(!subject->symbolView->isActive());
    subject->symbolView->showSymbolView();
    QVERIFY(subject->symbolView->isActive());
    QCOMPARE(subject->symbolView->currentLevel(), 0);
    QCOMPARE(subject->vkbWidget->shiftStatus(), DuiVirtualKeyboard::ShiftOff);

    subject->vkbWidget->setShiftState(DuiVirtualKeyboard::ShiftOn);
    QCOMPARE(subject->vkbWidget->shiftStatus(), DuiVirtualKeyboard::ShiftOn);
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(subject->symbolView->currentLevel(), 1);

    subject->vkbWidget->setShiftState(DuiVirtualKeyboard::ShiftLock);
    QCOMPARE(spy1.count(), 2);
    QCOMPARE(subject->vkbWidget->shiftStatus(), DuiVirtualKeyboard::ShiftLock);
    QCOMPARE(subject->symbolView->currentLevel(), 1);

    subject->vkbWidget->setShiftState(DuiVirtualKeyboard::ShiftOff);
    QCOMPARE(spy1.count(), 3);
    QCOMPARE(subject->vkbWidget->shiftStatus(), DuiVirtualKeyboard::ShiftOff);
    QCOMPARE(subject->symbolView->currentLevel(), 0);

    subject->hide();
}

void Ut_DuiKeyboardHost::testKeyCycle_data()
{
    QTest::addColumn<bool>("accurateMode");
    QTest::addColumn<QString>("preedit");

    QTest::newRow("fast mode") << false << "";
    QTest::newRow("fast mode, preedit") << false << "preedit";
    QTest::newRow("accurate mode") << true << "";
}

void Ut_DuiKeyboardHost::testKeyCycle()
{
    QFETCH(bool, accurateMode);
    QFETCH(QString, preedit);
    QString text = "123";
    KeyEvent event1(text,  QEvent::KeyRelease, Qt::Key_unknown, KeyEvent::CycleSet);
    KeyEvent event2("456", QEvent::KeyRelease, Qt::Key_unknown, KeyEvent::CycleSet);
    KeyEvent space ( " ",  QEvent::KeyRelease, Qt::Key_Space);

    //this value must be greater that MultitapTime in the file duikeyboardhost.cpp
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

QTEST_APPLESS_MAIN(Ut_DuiKeyboardHost);
