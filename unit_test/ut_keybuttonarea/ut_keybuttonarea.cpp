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

#include "ut_keybuttonarea.h"
#include "mvirtualkeyboardstyle.h"
#include "singlewidgetbuttonarea.h"
#include "singlewidgetbutton.h"
#include "flickupbutton.h"
#include "mbuttonarea.h"
#include "keyboarddata.h"
#include "vkbdatakey.h"
#include "mplainwindow.h"
#include "popupbase.h"

#include <MApplication>
#include <MScene>
#include <MSceneManager>
#include <MTheme>

#include <QDir>
#include <QGraphicsLayout>
#include <QGraphicsSceneMouseEvent>
#include <QTouchEvent>

#include <algorithm>

namespace
{
    const int LongPressTime = 600; // same as in keybuttonarea.cpp
}

Q_DECLARE_METATYPE(KeyEvent);
Q_DECLARE_METATYPE(IKeyButton*);
Q_DECLARE_METATYPE(const IKeyButton*);

typedef KeyButtonArea *(*KBACreator)(MVirtualKeyboardStyleContainer *styleContainer,
                                     QSharedPointer<const LayoutSection> section,
                                     KeyButtonArea::ButtonSizeScheme buttonSizeScheme,
                                     bool usePopup,
                                     QGraphicsWidget *parent);

Q_DECLARE_METATYPE(KBACreator);

KeyButtonArea *createSingleWidgetKeyButtonArea(MVirtualKeyboardStyleContainer *styleContainer,
                                               QSharedPointer<const LayoutSection> section,
                                               KeyButtonArea::ButtonSizeScheme buttonSizeScheme = KeyButtonArea::ButtonSizeEqualExpanding,
                                               bool usePopup = false,
                                               QGraphicsWidget *parent = 0)
{
    return new SingleWidgetButtonArea(styleContainer, section, buttonSizeScheme, usePopup, parent);
}

KeyButtonArea *createMButtonArea(MVirtualKeyboardStyleContainer *styleContainer,
                                   QSharedPointer<const LayoutSection> section,
                                   KeyButtonArea::ButtonSizeScheme buttonSizeScheme = KeyButtonArea::ButtonSizeEqualExpanding,
                                   bool usePopup = false,
                                   QGraphicsWidget *parent = 0)
{
    return new MButtonArea(styleContainer, section, buttonSizeScheme, usePopup, parent);
}


void Ut_KeyButtonArea::initTestCase()
{
    static int argc = 1;
    static char *app_name[1] = { (char *) "ut_keybuttonarea" };

    // Avoid waiting if im server is not responding
    MApplication::setLoadMInputContext(false);
    app = new MApplication(argc, app_name);

    MTheme::instance()->loadCSS("/usr/share/meegotouch/virtual-keyboard/css/864x480.css");
    style = new MVirtualKeyboardStyleContainer;
    style->initialize("MVirtualKeyboard", "MVirtualKeyboardView", 0);

    qRegisterMetaType<KeyEvent>();
    qRegisterMetaType<const IKeyButton*>();
    qRegisterMetaType<IKeyButton*>();

    new MPlainWindow; // Create singleton
}

void Ut_KeyButtonArea::cleanupTestCase()
{
    delete MPlainWindow::instance();
    delete style;
    style = 0;
    delete app;
    app = 0;
}

void Ut_KeyButtonArea::init()
{
    subject = 0;
    keyboard = 0;
}

void Ut_KeyButtonArea::cleanup()
{
    delete subject;
    delete keyboard;
    keyboard = 0;
}

void Ut_KeyButtonArea::testLandscapeBoxSize_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
    QTest::newRow("MButtonArea") << &createMButtonArea;
}

void Ut_KeyButtonArea::testLandscapeBoxSize()
{
    QFETCH(KBACreator, createKba);

    QSize box;
    QDir dir("/usr/share/meegotouch/virtual-keyboard/layouts/");
    QStringList filters;
    int fileCount = 0;

    changeOrientation(M::Angle0);

    filters << "??.xml";
    foreach(QFileInfo info, dir.entryInfoList(filters)) {
        delete subject;
        subject = 0;
        delete keyboard;
        keyboard = new KeyboardData;
        qDebug() << "Loading layout file" << info.fileName();
        QVERIFY(keyboard->loadNokiaKeyboard(info.fileName()));
        subject = createKba(style,
                            keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                            KeyButtonArea::ButtonSizeEqualExpanding,
                            false, 0);
        MPlainWindow::instance()->scene()->addItem(subject);

        subject->resize(defaultLayoutSize());

        for (int level = 0; level < 2; level++) {
            subject->switchLevel(level);

            box = keyAt(0, 0)->buttonRect().size();
            qDebug() << "Current level" << level << "; Box size=" << box;
            QVERIFY(box.height() >= 55 && box.height() <= 80);
            QVERIFY(box.width() >= 70 && box.width() <= 86);
        }
        ++fileCount;
    }
    QVERIFY(fileCount > 0);
}

void Ut_KeyButtonArea::testPortraitBoxSize_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
    QTest::newRow("MButtonArea") << &createMButtonArea;
}

void Ut_KeyButtonArea::testPortraitBoxSize()
{
    QFETCH(KBACreator, createKba);

    QSize box;
    QDir dir("/usr/share/meegotouch/virtual-keyboard/layouts/");
    QStringList filters;
    int fileCount = 0;

    changeOrientation(M::Angle90);

    filters << "??.xml";
    foreach(QFileInfo info, dir.entryInfoList(filters)) {
        delete subject;
        subject = 0;
        delete keyboard;
        keyboard = new KeyboardData;
        qDebug() << "Loading layout file" << info.fileName();
        QVERIFY(keyboard->loadNokiaKeyboard(info.fileName()));
        subject = createKba(style, keyboard->layout(LayoutData::General, M::Portrait)->section(LayoutData::mainSection),
                            KeyButtonArea::ButtonSizeEqualExpanding,
                            false, 0);
        MPlainWindow::instance()->scene()->addItem(subject);

        subject->resize(defaultLayoutSize());
        for (int level = 0; level < 2; level++) {
            subject->switchLevel(level);

            box = keyAt(0, 0)->buttonRect().size();
            qDebug() << "Current level" << level << "; Box size=" << box << subject->size();
            QVERIFY(box.height() >= 60 && box.height() <= 80);
            QVERIFY(box.width() >= 35 && box.width() <= 50);
        }
        ++fileCount;
    }
    QVERIFY(fileCount > 0);
}

void Ut_KeyButtonArea::testLabelPosition_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
    QTest::newRow("MButtonArea") << &createMButtonArea;
}

void Ut_KeyButtonArea::testLabelPosition()
{
    QFETCH(KBACreator, createKba);

    QList<QPoint> positions;
    QList<const VKBDataKey *> outcome;
    QSize buttonSize;
    int startX;
    const int rowIndex = 2;
    const IKeyButton *button = 0;

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en.xml"));
    subject = createKba(style, keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        KeyButtonArea::ButtonSizeEqualExpanding,
                        false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);

    subject->resize(defaultLayoutSize());

    const IKeyButton *firstButton = keyAt(0, 0);

    button = keyAt(2, 0); // Third row's first button
    buttonSize = button->buttonBoundingRect().size();
    startX = button->buttonBoundingRect().x();
    positions << QPoint(-50, -50)                       // not on any key
              << firstButton->buttonRect().topLeft() + QPoint(-1, -1) // reactive margin should get this
              << QPoint(-buttonSize.width(), 0)         // not on any key
              << QPoint(0, -buttonSize.height())        // not on any key
              << QPoint(1, 100 * buttonSize.height())   // not on any key
              << QPoint(startX + buttonSize.width() + (*style)->spacingHorizontal() + 1,
                        (*style)->paddingTop() + (buttonSize.height() + (*style)->spacingVertical()) * rowIndex + 1)
              << QPoint(startX + buttonSize.width() * 100 + 1, buttonSize.height() * rowIndex + 1); // not on any key
    outcome << 0
            << subject->section->getVKBKey(0, 0)
            << 0
            << 0
            << 0
            << subject->section->getVKBKey(rowIndex, 1)
            << 0;
    QVERIFY(positions.count() == outcome.count());
    for (int n = 0; n < positions.count(); ++n) {
        qDebug() << "test position" << positions.at(n);
        button = subject->keyAt(positions.at(n));
        const VKBDataKey *result = (button ? &button->key() : 0);
        QCOMPARE(result, outcome.at(n));
    }
}

void Ut_KeyButtonArea::testFlickCheck_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::addColumn<bool>("directMode");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea << false;
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea << true;
    QTest::newRow("MButtonArea") << &createMButtonArea << false;
    QTest::newRow("MButtonArea") << &createMButtonArea << true;
}

void Ut_KeyButtonArea::testFlickCheck()
{
    QFETCH(KBACreator, createKba);
    QFETCH(bool, directMode);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en.xml"));
    subject = createKba(style, keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        KeyButtonArea::ButtonSizeEqualExpanding,
                        false, 0);
    KeyButtonArea::setInputMethodMode(directMode ? M::InputMethodModeDirect : M::InputMethodModeNormal);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    // A series of touch points that will not create a gesture yet:
    PointList base;
    for (int idx = 0; idx < 8; ++idx) {
        // But each new point has to overcome the movement threshold:
        base << QPoint(idx * 6, idx * 6); 
    }
    recognizeGesture(base, NoGesture);

    PointList rightSwipe0 = PointList(base);
    rightSwipe0 << QPoint(600, 0);
    recognizeGesture(rightSwipe0, directMode ? NoGesture : SwipeRightGesture);
    recognizeGesture(reversed(rightSwipe0), directMode ? NoGesture : SwipeLeftGesture);

    // A swipe does not have to be a strictly increasing sequence:
    PointList rightSwipe1 = PointList(base);
    rightSwipe1 << QPoint(0, 0) << QPoint(600, 0);
    recognizeGesture(rightSwipe1, directMode ? NoGesture : SwipeRightGesture);
    recognizeGesture(reversed(rightSwipe1), directMode ? NoGesture : SwipeLeftGesture);

    PointList downSwipe = PointList(base);
    downSwipe << QPoint(0, 800);
    recognizeGesture(downSwipe, directMode ? NoGesture : SwipeDownGesture);
    recognizeGesture(reversed(downSwipe), NoGesture); // No key set - can't swipe up ...

    PointList conflictingSwipe = PointList(base);
    conflictingSwipe << QPoint(800, 800);
    recognizeGesture(conflictingSwipe, NoGesture);
}

void Ut_KeyButtonArea::testSceneEvent_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
    QTest::newRow("MButtonArea") << &createMButtonArea;
}

void Ut_KeyButtonArea::testSceneEvent()
{
    QFETCH(KBACreator, createKba);

    //initialization
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en.xml"));
    subject = createKba(style, keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        KeyButtonArea::ButtonSizeEqualExpanding,
                        false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    // Skip test for multitouch, since there are no mouse events:
    if (subject->acceptTouchEvents()) {
        return;
    }

    QGraphicsSceneMouseEvent *press = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMousePress);
    QGraphicsSceneMouseEvent *release = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMouseRelease);
    QGraphicsSceneMouseEvent *move = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMouseMove);
    QSignalSpy spy(subject, SIGNAL(keyClicked(const IKeyButton*, const QString&, bool)));
    QSignalSpy spyPressed(subject, SIGNAL(keyPressed(const IKeyButton*, const QString&, bool)));

    QVERIFY(spy.isValid());
    QVERIFY(spyPressed.isValid());

    press->setPos(QPoint(1, 1));
    release->setPos(QPoint(10, 10));
    move->setPos(QPoint(10, 10));

    //actual testing
    subject->sceneEvent(press);
    QCOMPARE(spyPressed.count(), 1);
    QCOMPARE(spy.count(), 0);
    subject->sceneEvent(move);
    QCOMPARE(spy.count(), 0);
    subject->sceneEvent(release);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spyPressed.count(), 1);

    delete press;
    delete move;
    delete release;
}

void Ut_KeyButtonArea::testPaint_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
    QTest::newRow("MButtonArea") << &createMButtonArea;
}

void Ut_KeyButtonArea::testPaint()
{
    QFETCH(KBACreator, createKba);

    //at least we should not chrash here
    QImage *image = new QImage(QSize(864, 480), QImage::Format_ARGB32_Premultiplied);
    QPainter painter;
    QVERIFY(painter.begin(image));

    //initialization
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en.xml"));
    subject = createKba(style, keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        KeyButtonArea::ButtonSizeEqualExpanding,
                        false, 0);
    subject->resize(defaultLayoutSize());
    MPlainWindow::instance()->scene()->addItem(subject);

    subject->touchPointPressed(QPoint(20, 20), 0); // top left button
    //actual testing
    subject->paint(&painter, 0, 0);
}

void Ut_KeyButtonArea::testDeadkeys_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
    QTest::newRow("MButtonArea") << &createMButtonArea;
}

void Ut_KeyButtonArea::testDeadkeys()
{
    QFETCH(KBACreator, createKba);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("fr.xml"));
    subject = createKba(style, keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        KeyButtonArea::ButtonSizeEqualExpanding,
                        false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());
    QSignalSpy spy(subject, SIGNAL(keyClicked(const IKeyButton*, const QString&, bool)));
    const IKeyButton *key = 0;
    QList<int> positions;
    int i;
    positions << 0 << 1 << 2 << 5 << 6;

    QVERIFY(spy.isValid());

    //!!! z is the character that won't be changed when deadkey is locked
    QStringList lowerUnicodes;
    lowerUnicodes << QChar('a') << QChar('z') << QChar('e') << QChar('y') << QChar('u');
    QStringList upperUnicodes; //upper case
    foreach(QString c, lowerUnicodes)
    upperUnicodes << c.toUpper();

    //Unicode for alphas with deadkeys:  á << Unicod é  unicode for  ý<<< unicode for  ú
    QStringList lowerDKUnicodes;
    lowerDKUnicodes << QChar(0x00e1) << QChar('z') << QChar(0x00e9) << QChar(0x00fd) << QChar(0x00fa);
    QStringList upperDKUnicodes; // upper case
    foreach(QString c, lowerDKUnicodes)
    upperDKUnicodes << c.toUpper();

    //test for unlock deadkey status
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerUnicodes.at(i));
    }

    key = keyAt(2, 7); // row 3, column 8
    QVERIFY(key != 0);
    QVERIFY(key->isDeadKey());
    QString c = QChar(0x00B4);
    QCOMPARE(key->label(), c);
    subject->click(key);
    //click at deadkey for the first time, just lock the deadkey, won't emit cliked() signal
    QCOMPARE(spy.count(), 0);

    //check the alphas, should be with deadkey
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerDKUnicodes.at(i));
    }

    //test for shift status
    subject->switchLevel(1);
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), upperDKUnicodes.at(i));
    }

    //after unlock the dead key, test the shift status
    subject->unlockDeadkeys();
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), upperUnicodes.at(i));
    }

    //test for shift off status
    subject->switchLevel(0);
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerUnicodes.at(i));
    }

    // Lock deadkey again.
    subject->clickAtDeadkey(key);
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerDKUnicodes.at(i));
    }

    subject->click(keyAt(0, 0));
    //key release on not deadkey, will emit clicked() signal
    QCOMPARE(spy.count(), 1);
    //any keypress, the deadkey should be unlocked
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerUnicodes.at(i));
    }
}

void Ut_KeyButtonArea::testImportedLayouts_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
    QTest::newRow("MButtonArea") << &createMButtonArea;
}

void Ut_KeyButtonArea::testImportedLayouts()
{
    QFETCH(KBACreator, createKba);

    // This test uses files test-importer.xml, test-import1.xml, and test-import2.xml.
    // The first imported file test-import1.xml defines some landscape and portrait stuff, while
    // the second imported file test-import2.xml redefines the portrait stuff.

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("test-importer.xml"));
    const LayoutData *model = keyboard->layout(LayoutData::General, M::Landscape);
    QVERIFY(model);
    subject = createKba(style, model->section(LayoutData::mainSection),
                        KeyButtonArea::ButtonSizeEqualExpanding,
                        false, 0);

    // Check that language and title information are not overwritten by imported files.
    QCOMPARE(keyboard->title(), QString("test-importer-title"));
    QCOMPARE(keyboard->language(), QString("test-importer-language"));

    // First imported defines landscape layout with key labeled "1".
    // Second one doesn't define landscape layout.
    changeOrientation(M::Angle0);
    QCOMPARE(keyAt(0, 0)->label(), QString("1"));
    QCOMPARE(model->section(LayoutData::functionkeySection)->getVKBKey(0, 0)->binding(false)->label(),
             QString("func1"));

    // Second imported defines portrait layout with key labeled "2"
    // and also it defines portrait functionkeys which overwrites
    // the first one's functionkeys.
    delete subject;
    model = keyboard->layout(LayoutData::General, M::Portrait);
    subject = createKba(style, model->section(LayoutData::mainSection),
                        KeyButtonArea::ButtonSizeEqualExpanding,
                        false, 0);
    changeOrientation(M::Angle90);
    QCOMPARE(keyAt(0, 0)->label(), QString("2"));
    QCOMPARE(model->section(LayoutData::functionkeySection)->getVKBKey(0, 0)->binding(false)->label(),
             QString("func2"));
}

void Ut_KeyButtonArea::testAccurateMode_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
    QTest::newRow("MButtonArea") << &createMButtonArea;
}

void Ut_KeyButtonArea::testAccurateMode()
{
    QFETCH(KBACreator, createKba);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en.xml"));
    subject = createKba(style, keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        KeyButtonArea::ButtonSizeEqualExpanding, true, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    subject->accurateStop();
    QVERIFY(!subject->isAccurateMode());

    // Test starting accurate mode

    // direct call
    subject->accurateStart();
    QVERIFY(subject->isAccurateMode());
    subject->accurateStop();

    // make long press

    const QPoint mousePos(20, 20); // approximately the top left key on layout
    const int touchId = 0;

    subject->touchPointPressed(mousePos, touchId);
    QTest::qWait(LongPressTime - 100); // not enough time
    subject->touchPointReleased(mousePos, touchId);
    QVERIFY(!subject->isAccurateMode());

    subject->touchPointPressed(mousePos, touchId);
    QTest::qWait(LongPressTime + 100); // long enough

    // When accurate mode is on and mouse down we should have popup enabled
    QVERIFY(subject->isPopupActive());

    subject->touchPointReleased(mousePos, touchId);
    QVERIFY(subject->isAccurateMode());
}

void Ut_KeyButtonArea::testPopup_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
    QTest::newRow("MButtonArea") << &createMButtonArea;
}

void Ut_KeyButtonArea::testPopup()
{
    QFETCH(KBACreator, createKba);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en.xml"));
    subject = createKba(style, keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        KeyButtonArea::ButtonSizeEqualExpanding, true, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    const QPoint mousePos(20, 20); // approximately the top left key on layout
    const int touchId = 0;

    // Test popup activation

    // direct call

    // Popup won't show up unless it is given a position. We give it via a mouse press.
    subject->touchPointPressed(mousePos, touchId);
    subject->popupStart();
    QVERIFY(subject->isPopupActive());
    subject->popup->hidePopup();
    QVERIFY(!subject->isPopupActive());
    subject->touchPointReleased(mousePos, touchId);

    // make long press

    subject->touchPointPressed(mousePos, touchId);
    QTest::qWait(LongPressTime - 100); // not enough time
    QVERIFY(!subject->isPopupActive());
    subject->touchPointReleased(mousePos, touchId);

    subject->touchPointPressed(mousePos, touchId);
    QTest::qWait(LongPressTime + 100); // long enough
    QVERIFY(subject->isPopupActive());
    subject->touchPointReleased(mousePos, touchId);
}

void Ut_KeyButtonArea::testInitialization_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
    QTest::newRow("MButtonArea") << &createMButtonArea;
}

void Ut_KeyButtonArea::testInitialization()
{
    QFETCH(KBACreator, createKba);
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en.xml"));
    subject = createKba(style, keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        KeyButtonArea::ButtonSizeEqualExpanding,
                        false, 0);
    subject->resize(defaultLayoutSize());

}

void Ut_KeyButtonArea::testFunctionRowAlignmentBug_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
    QTest::newRow("MButtonArea") << &createMButtonArea;
}

// This tests the bug where function row alignment in number portrait keyboard
// didn't respect the right alignment attribute read from xml.
void Ut_KeyButtonArea::testFunctionRowAlignmentBug()
{
    QFETCH(KBACreator, createKba);
    QGraphicsScene scene(QRectF(0, 0, 1000, 1000));

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("number.xml"));
    const LayoutData *numberLayout = keyboard->layout(LayoutData::Number, M::Portrait);
    QVERIFY(numberLayout);
    QSharedPointer<const LayoutSection> functionRowSection = numberLayout->section(LayoutData::functionkeySection);

    if (!(functionRowSection->horizontalAlignment() & Qt::AlignRight)) {
        QSKIP("Skipping because the loaded number layout is not right-aligned.", SkipSingle);
    }

    // set equalButtonWidth = false, so button's own width is used
    subject = createKba(style, functionRowSection, KeyButtonArea::ButtonSizeFunctionRow, false, 0);
    scene.addItem(subject);

    // The size is not important as long as there's space for about two buttons.
    const int width = 500;
    subject->resize(width, 1); // KeyButtonArea ignores height

    const QPoint buttonLeftPos = QPoint(20, 20);
    const QPoint buttonRightPos = QPoint(width - 20, 20);

    const bool buttonFoundFromLeft = (subject->keyAt(buttonLeftPos) != 0);
    const bool buttonFoundFromRight = (subject->keyAt(buttonRightPos) != 0);

    scene.removeItem(subject);

    QVERIFY(!buttonFoundFromLeft); // button should not be found from left side
    QVERIFY(buttonFoundFromRight); // button should be found from right side
}

void Ut_KeyButtonArea::testShiftCapsLock()
{
    // Load any layout that has function row with shift
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en.xml"));
    const LayoutData *layout = keyboard->layout(LayoutData::General, M::Landscape);
    QVERIFY(layout);
    QSharedPointer<const LayoutSection> functionRowSection = layout->section(LayoutData::functionkeySection);

    subject = createSingleWidgetKeyButtonArea(style, functionRowSection,
                                              KeyButtonArea::ButtonSizeFunctionRow,
                                              false, 0);

    SingleWidgetButton *shiftButton = static_cast<SingleWidgetButtonArea *>(subject)->shiftButton;
    QVERIFY(shiftButton);
    QVERIFY(shiftButton->state() == IKeyButton::Normal);

    subject->setShiftStatus(true, true);
    QVERIFY(shiftButton->state() == IKeyButton::Selected);

    subject->setShiftStatus(true, false);
    QVERIFY(shiftButton->state() == IKeyButton::Normal);
}

void Ut_KeyButtonArea::testMultiTouch()
{
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en.xml"));
    const LayoutData *layout = keyboard->layout(LayoutData::General, M::Landscape);
    QVERIFY(layout);
    QSharedPointer<const LayoutSection> functionRowSection = layout->section(LayoutData::mainSection);

    subject = createSingleWidgetKeyButtonArea(style, functionRowSection,
                                              KeyButtonArea::ButtonSizeFunctionRow,
                                              false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    const IKeyButton *key0 = keyAt(0, 0);
    const IKeyButton *key1 = keyAt(1, 0);
    const IKeyButton *key2 = keyAt(0, 1);

    QVERIFY(key0);
    QVERIFY(key1);
    QVERIFY(key2);

    QSignalSpy pressed(subject, SIGNAL(keyPressed(const IKeyButton*, const QString&, bool)));
    QSignalSpy released(subject, SIGNAL(keyReleased(const IKeyButton*, const QString&, bool)));
    QSignalSpy clicked(subject, SIGNAL(keyClicked(const IKeyButton*, const QString&, bool )));

    QVERIFY(pressed.isValid());
    QVERIFY(released.isValid());
    QVERIFY(clicked.isValid());

    const QPoint pos0 = key0->buttonRect().center();
    const QPoint pos1 = key1->buttonRect().center();
    const QPoint pos2 = key2->buttonRect().center();

    /*
     * Verify following conditions:
     * 1) signals are emitted in correct order
     * 2) every signal corresponds to correct key
     */
    subject->touchPointPressed(pos0, 0);
    QCOMPARE(pressed.count(), 1);
    QVERIFY(pressed.at(0).first().value<const IKeyButton*>() == key0);
    QCOMPARE(released.count(), 0);
    QCOMPARE(clicked.count(), 0);

    subject->touchPointPressed(pos1, 1);
    QCOMPARE(pressed.count(), 2);
    QVERIFY(pressed.at(1).first().value<const IKeyButton*>() == key1);
    QCOMPARE(released.count(), 0);
    QCOMPARE(clicked.count(), 0);

    subject->touchPointReleased(pos0, 0);
    subject->touchPointReleased(pos1, 1);
    QCOMPARE(pressed.count(), 2);
    QCOMPARE(released.count(), 2);
    QVERIFY(released.at(0).first().value<const IKeyButton*>() == key0);
    QVERIFY(released.at(1).first().value<const IKeyButton*>() == key1);
    QCOMPARE(clicked.count(), 2);
    QVERIFY(clicked.at(0).first().value<const IKeyButton*>() == key0);
    QVERIFY(clicked.at(1).first().value<const IKeyButton*>() == key1);

    pressed.clear();
    released.clear();
    clicked.clear();

    // Verify if could click on some keys while other key is pressed
    subject->touchPointPressed(pos0, 0);
    subject->touchPointPressed(pos1, 1);
    subject->touchPointReleased(pos0, 0);
    subject->touchPointPressed(pos2, 0);
    subject->touchPointReleased(pos2, 0);
    subject->touchPointReleased(pos1, 1);

    QCOMPARE(pressed.count(), 3);
    QCOMPARE(released.count(), 3);
    QCOMPARE(clicked.count(), 3);

    QVERIFY(pressed.at(0).first().value<const IKeyButton*>() == key0);
    QVERIFY(pressed.at(1).first().value<const IKeyButton*>() == key1);
    QVERIFY(pressed.at(2).first().value<const IKeyButton*>() == key2);

    QVERIFY(released.at(0).first().value<const IKeyButton*>() == key0);
    QVERIFY(released.at(1).first().value<const IKeyButton*>() == key2);
    QVERIFY(released.at(2).first().value<const IKeyButton*>() == key1);

    QVERIFY(clicked.at(0).first().value<const IKeyButton*>() == key0);
    QVERIFY(clicked.at(1).first().value<const IKeyButton*>() == key2);
    QVERIFY(clicked.at(2).first().value<const IKeyButton*>() == key1);
}

void Ut_KeyButtonArea::changeOrientation(M::OrientationAngle angle)
{
    if (MPlainWindow::instance()->orientationAngle() != angle) {
        MPlainWindow::instance()->setOrientationAngle(angle);
        //timeout depends on duration of orientation animation
        QTest::qWait(1000);
    }
}

QSize Ut_KeyButtonArea::defaultLayoutSize()
{
    // Take visible scene size as layout size, but reduce keyboard's paddings first from its width.
    // The height value is ignored since KeyButtonAreas determine their own height.
    return MPlainWindow::instance()->visibleSceneSize()
            - QSize((*style)->paddingLeft() + (*style)->paddingRight(), 0);
}

// Helper method to get key in certain row and column from current subject.
const IKeyButton *Ut_KeyButtonArea::keyAt(unsigned int row, unsigned int column) const
{
    // If this fails there is something wrong with the test.
    Q_ASSERT(subject
             && (row < static_cast<unsigned int>(subject->rowCount()))
             && (column < static_cast<unsigned int>(subject->sectionModel()->columnsAt(row))));

    const IKeyButton *key = 0;

    if (dynamic_cast<MButtonArea *>(subject)) {
        MButtonArea *buttonArea = static_cast<MButtonArea *>(subject);
        int keyIndex = column;
        for (unsigned int i = 0; i < row; ++i) {
            keyIndex += buttonArea->sectionModel()->columnsAt(i);
        }
        key = buttonArea->buttons[keyIndex];
    } else { // assume SingleWidgetButtonArea
        SingleWidgetButtonArea *buttonArea = static_cast<SingleWidgetButtonArea *>(subject);
        key = buttonArea->rowList[row].buttons[column];
    }

    return key;
}

void Ut_KeyButtonArea::recognizeGesture(const PointList &pl, GestureType gt, int touchPointId)
{
    QSignalSpy leftSwipeSpy(subject, SIGNAL(flickLeft()));
    QSignalSpy rightSwipeSpy(subject, SIGNAL(flickRight()));
    QSignalSpy upSwipeSpy(subject, SIGNAL(flickUp(KeyBinding)));
    QSignalSpy downSwipeSpy(subject, SIGNAL(flickDown()));

    subject->touchPointPressed(pl[0], touchPointId);

    for (int n = 1; n < pl.count(); ++n) {
        subject->touchPointMoved(pl[n], touchPointId);
    }

    // Not needed for the gesture, but we don't want to leave a pressed touch
    // point without a release:
    subject->touchPointReleased(pl[pl.count() - 1], touchPointId);

    qDebug() << "gesture type = " << gt;
    QCOMPARE(leftSwipeSpy.count(),
             gt == SwipeLeftGesture ? 1 : 0);
    QCOMPARE(rightSwipeSpy.count(),
             gt == SwipeRightGesture ? 1 : 0);
    QCOMPARE(upSwipeSpy.count(),
             gt == SwipeUpGesture ? 1 : 0);
    QCOMPARE(downSwipeSpy.count(),
             gt == SwipeDownGesture ? 1 : 0);
}


Ut_KeyButtonArea::PointList Ut_KeyButtonArea::reversed(const PointList &in) const {
    PointList result;
    std::reverse_copy(in.begin(), in.end(), std::back_inserter(result));
    return result;
}
QTEST_APPLESS_MAIN(Ut_KeyButtonArea);
