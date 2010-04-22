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

namespace
{
    const int LongPressTime = 1000; // same as in keybuttonarea.cpp
}

Q_DECLARE_METATYPE(KeyEvent);

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
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
    QTest::newRow("MButtonArea") << &createMButtonArea;
}

void Ut_KeyButtonArea::testFlickCheck()
{
    QFETCH(KBACreator, createKba);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en.xml"));
    subject = createKba(style, keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        KeyButtonArea::ButtonSizeEqualExpanding,
                        false, 0);
    subject->resize(defaultLayoutSize());

    QList<QPointF> positions;
    QList<int> left;
    QList<int> right;
    QList<int> down;
    QList<int> up;
    QList<bool> outcome;
    QSignalSpy spyLeft(subject, SIGNAL(flickLeft()));
    QSignalSpy spyRight(subject, SIGNAL(flickRight()));
    QSignalSpy spyDown(subject, SIGNAL(flickDown()));
    QSignalSpy spyUp(subject, SIGNAL(flickUp(const KeyBinding *)));
    bool result;

    QVERIFY(spyLeft.isValid());
    QVERIFY(spyRight.isValid());
    QVERIFY(spyDown.isValid());
    QVERIFY(spyUp.isValid());

    positions << QPointF(0, 0)
              << QPointF(25, 25)
              << QPointF(75, 75)
              << QPointF(25, -75)
              << QPointF(-75, 5)
              << QPointF(75, 5)
              << QPointF(0, 75);
    left  << 0 << 0 << 0 << 0 << 0 << 1 << 0;
    right << 0 << 0 << 0 << 0 << 1 << 0 << 0;
    down  << 0 << 0 << 0 << 1 << 0 << 0 << 0;
    up    << 0 << 0 << 0 << 0 << 0 << 0 << 1;
    outcome << false << false << false
            << true << true << true
            << true;

    QVERIFY(positions.count() == outcome.count());
    QVERIFY(positions.count() == left.count());
    subject->pointerPos = QPoint(0, 0);

    for (int n = 0; n < positions.count(); ++n) {
        spyLeft.clear();
        spyRight.clear();
        spyDown.clear();
        spyUp.clear();
        qDebug() << "test position" << positions.at(n);
        subject->flickStartPos = positions.at(n);
        subject->flicked = false;
        result = subject->flickCheck();
        QCOMPARE(result, outcome.at(n));
        QCOMPARE(left.at(n), spyLeft.count());
        QCOMPARE(right.at(n), spyRight.count());
        QCOMPARE(down.at(n), spyDown.count());
        QCOMPARE(up.at(n), spyUp.count());
    }
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

    QGraphicsSceneMouseEvent *press = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMousePress);
    QGraphicsSceneMouseEvent *release = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMouseRelease);
    QGraphicsSceneMouseEvent *move = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMouseMove);
    QSignalSpy spy(subject, SIGNAL(keyClicked(const KeyEvent &)));
    QSignalSpy spyPressed(subject, SIGNAL(keyPressed(const KeyEvent &)));

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
    QVERIFY(painter.begin(image) == true);

    //initialization
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en.xml"));
    subject = createKba(style, keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        KeyButtonArea::ButtonSizeEqualExpanding,
                        false, 0);
    subject->resize(defaultLayoutSize());
    subject->pointerPos = QPoint(20, 20); // top left button
    subject->fingerInsideArea = true;
    subject->accurateStart();
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
    QSignalSpy spy(subject, SIGNAL(keyClicked(const KeyEvent &)));
    const IKeyButton *key = 0;
    QList<int> positions;
    int i;
    positions << 0 << 1 << 2 << 5 << 6;

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
    subject->clickAtDeadkey(key);
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

    QGraphicsSceneMouseEvent *release = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMouseRelease);
    release->setPos(QPointF(keyAt(0, 0)->buttonBoundingRect().x() + 1.0, 1.0)); // on some valid key
    subject->mouseReleaseEvent(release);
    //key release on not deadkey, will emit clicked() signal
    QCOMPARE(spy.count(), 1);
    //any keypress, the deadkey should be unlocked
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerUnicodes.at(i));
    }
    delete release;
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
    QGraphicsSceneMouseEvent mouseEvent;
    mouseEvent.setPos(QPointF(20.0f, 20.0f)); // approximately the top left key on layout

    subject->mousePressEvent(&mouseEvent);
    QTest::qWait(LongPressTime - 100); // not enough time
    subject->mouseReleaseEvent(&mouseEvent);
    QVERIFY(!subject->isAccurateMode());

    subject->mousePressEvent(&mouseEvent);
    QTest::qWait(LongPressTime + 100); // long enough

    // When accurate mode is on and mouse down we should have popup enabled
    QVERIFY(subject->isPopupActive());

    subject->mouseReleaseEvent(&mouseEvent);
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

    // Test popup activation

    // direct call
    subject->popupStart();
    QVERIFY(subject->isPopupActive());
    subject->popup->hidePopup();
    QVERIFY(!subject->isPopupActive());

    // make long press
    QGraphicsSceneMouseEvent mouseEvent;
    mouseEvent.setPos(QPointF(20.0f, 20.0f)); // approximately the top left key on layout

    subject->mousePressEvent(&mouseEvent);
    QTest::qWait(LongPressTime - 100); // not enough time
    subject->mouseReleaseEvent(&mouseEvent);
    QVERIFY(!subject->isPopupActive());

    subject->mousePressEvent(&mouseEvent);
    QTest::qWait(LongPressTime + 100); // long enough
    QVERIFY(subject->isPopupActive());
    subject->mouseReleaseEvent(&mouseEvent);
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


QTEST_APPLESS_MAIN(Ut_KeyButtonArea);
