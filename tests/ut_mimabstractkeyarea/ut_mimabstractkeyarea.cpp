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

#include "ut_mimabstractkeyarea.h"
#include "mimkeyarea.h"
#include "mimkey.h"
#include "flickgesturerecognizer.h"
#include "../ut_flickrecognizer/flickutil.h"
#include "keyboarddata.h"
#include "mimkeymodel.h"
#include "mplainwindow.h"
#include "popupbase.h"
#include "utils.h"
#include <regiontracker.h>

#include <MApplication>
#include <MScene>
#include <MSceneWindow>
#include <MSceneManager>
#include <MTheme>

#include <QDir>
#include <QGraphicsLayout>
#include <QGraphicsSceneMouseEvent>
#include <QTouchEvent>

#include <algorithm>

Q_DECLARE_METATYPE(KeyEvent);
Q_DECLARE_METATYPE(MImAbstractKey*);
Q_DECLARE_METATYPE(const MImAbstractKey*);

typedef MImAbstractKeyArea *(*KBACreator)(const LayoutData::SharedLayoutSection &section,
                                     bool usePopup,
                                     QGraphicsWidget *parent);

typedef QTouchEvent::TouchPoint (*TpCreator)(int id,
                                             Qt::TouchPointState state,
                                             const QPointF &pos,
                                             const QPointF &lastPos);

typedef QList<QTouchEvent::TouchPoint> TpList;
typedef QList<MImAbstractKey *> ButtonList;
typedef QList<MImAbstractKey::ButtonState> ButtonStateList;
typedef QList<ButtonStateList> TpButtonStateMatrix;

Q_DECLARE_METATYPE(KBACreator);
Q_DECLARE_METATYPE(TpCreator);

Q_DECLARE_METATYPE(TpList);
Q_DECLARE_METATYPE(ButtonList);
Q_DECLARE_METATYPE(ButtonStateList);
Q_DECLARE_METATYPE(TpButtonStateMatrix);
Q_DECLARE_METATYPE(QList<int>);

Q_DECLARE_METATYPE(MImAbstractKey::ButtonState);
Q_DECLARE_METATYPE(QList<MImKeyBinding::KeyAction>);
Q_DECLARE_METATYPE(Ut_MImAbstractKeyArea::TestOpList);

namespace {

    MImAbstractKeyArea *createKeyArea(const LayoutData::SharedLayoutSection &section,
                                      bool usePopup = false,
                                      QGraphicsWidget *parent = 0)
    {
        return new MImKeyArea(section, usePopup, parent);
    }
}

void Ut_MImAbstractKeyArea::initTestCase()
{
    static int argc = 2;
    static char *app_name[2] = { (char *) "ut_mimabstractkeyarea",
                                 (char *) "-software"};

    disableQtPlugins();
    app = new MApplication(argc, app_name);
    RegionTracker::createInstance();

    qRegisterMetaType<KeyEvent>();
    qRegisterMetaType<const MImAbstractKey*>();
    qRegisterMetaType<MImAbstractKey*>();
    qRegisterMetaType<MImAbstractKey::ButtonState>();
    qRegisterMetaType<TestOpList>("TestOpList");

    sceneWindow = createMSceneWindow(new MPlainWindow); // also create singleton

    FlickGestureRecognizer::registerSharedRecognizer();

    MPlainWindow::instance()->grabGesture(FlickGestureRecognizer::sharedGestureType());
    MPlainWindow::instance()->show();
    QTest::qWaitForWindowShown(MPlainWindow::instance());
}

void Ut_MImAbstractKeyArea::cleanupTestCase()
{
    FlickGestureRecognizer::unregisterSharedRecognizer();
    delete sceneWindow;
    delete MPlainWindow::instance();
    RegionTracker::destroyInstance();
    delete app;
    app = 0;
}

void Ut_MImAbstractKeyArea::init()
{
    subject = 0;
    keyboard = 0;
}

void Ut_MImAbstractKeyArea::cleanup()
{
    delete subject;
    delete keyboard;
    keyboard = 0;
}

void Ut_MImAbstractKeyArea::testLandscapeBoxSize_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createKeyArea;
}

void Ut_MImAbstractKeyArea::testLandscapeBoxSize()
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
        subject = createKba(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                            false, 0);
        MPlainWindow::instance()->scene()->addItem(subject);

        subject->resize(defaultLayoutSize());

        for (int level = 0; level < 2; level++) {
            subject->switchLevel(level);

            box = keyAt(0, 0)->buttonRect().size().toSize();
            QVERIFY(subject->sectionModel()->rowCount() > 0);
            QVERIFY(subject->sectionModel()->columnsAt(0) > 0);
            qDebug() << "Current level" << level << "; Box size=" << box;
            QVERIFY(box.height() >= 0 && box.height() <= (280/subject->sectionModel()->rowCount()));
            QVERIFY(box.width() >= 00 && box.width() <= (864/subject->sectionModel()->columnsAt(0)));
        }
        ++fileCount;
    }
    QVERIFY(fileCount > 0);
}

void Ut_MImAbstractKeyArea::testPortraitBoxSize_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createKeyArea;
}

void Ut_MImAbstractKeyArea::testPortraitBoxSize()
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
        subject = createKba(keyboard->layout(LayoutData::General, M::Portrait)->section(LayoutData::mainSection),
                            false, 0);
        MPlainWindow::instance()->scene()->addItem(subject);

        subject->resize(defaultLayoutSize());
        for (int level = 0; level < 2; level++) {
            subject->switchLevel(level);

            box = keyAt(0, 0)->buttonRect().size().toSize();
            qDebug() << "Current level" << level << "; Box size=" << box << subject->size();
            QVERIFY(box.height() >= 50 && box.height() <= 80);
            QVERIFY(box.width() >= 30 && box.width() <= 50);
        }
        ++fileCount;
    }
    QVERIFY(fileCount > 0);
}

void Ut_MImAbstractKeyArea::testSceneEvent_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createKeyArea;
}

void Ut_MImAbstractKeyArea::testSceneEvent()
{
    QSKIP("Overlapping testcase, remove?",
          SkipAll);

    QFETCH(KBACreator, createKba);

    //initialization
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));
    subject = createKba(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
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
    QSignalSpy spy(subject, SIGNAL(keyClicked(const MImAbstractKey*, const QString&, bool, const QPoint&)));
    QSignalSpy spyPressed(subject, SIGNAL(keyPressed(const MImAbstractKey*, const QString&, bool)));

    QVERIFY(spy.isValid());
    QVERIFY(spyPressed.isValid());

    press->setPos(QPoint(subject->style()->paddingTop() + 1, subject->style()->paddingLeft() + 1));
    release->setPos(QPoint(subject->style()->paddingTop() + 10, subject->style()->paddingLeft() + 10));
    move->setPos(QPoint(subject->style()->paddingTop() + 10, subject->style()->paddingLeft() + 10));

    //actual testing
    subject->sceneEvent(press);
    QCOMPARE(spyPressed.count(), 1);
    QCOMPARE(spy.count(), 0);
    subject->sceneEvent(move);
    QCOMPARE(spy.count(), 0);
    subject->sceneEvent(release);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst(); // take the first signal
    QCOMPARE(arguments.count(), 4);

    // verify the corrected touch point is within touched key button area.
    MImAbstractKey *key = keyAt(0, 0);
    QVERIFY(key->buttonRect().contains(arguments.at(3).toPoint()));

    QCOMPARE(spyPressed.count(), 1);

    delete press;
    delete move;
    delete release;
}

void Ut_MImAbstractKeyArea::testPaint_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createKeyArea;
}

void Ut_MImAbstractKeyArea::testPaint()
{
    QFETCH(KBACreator, createKba);

    //at least we should not chrash here
    QImage *image = new QImage(QSize(864, 480), QImage::Format_ARGB32_Premultiplied);
    QPainter painter;
    QVERIFY(painter.begin(image));

    //initialization
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));
    subject = createKba(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        false, 0);
    subject->resize(defaultLayoutSize());
    MPlainWindow::instance()->scene()->addItem(subject);

    //actual testing
    subject->paint(&painter, 0, 0);
}

void Ut_MImAbstractKeyArea::testDeadkeys_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createKeyArea;
}

void Ut_MImAbstractKeyArea::testDeadkeys()
{
    QFETCH(KBACreator, createKba);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("fr.xml"));
    subject = createKba(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());
    QSignalSpy spy(subject, SIGNAL(keyClicked(const MImAbstractKey*, const QString&, bool, const QPoint&)));
    MImAbstractKey *key = 0;
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

    key = keyAt(2, 8); // row 3, column 9
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
    SpecialKeyFinder finder(SpecialKeyFinder::FindDeadKey);
    MImAbstractKey::visitActiveKeys(&finder);

    subject->unlockDeadKeys(finder.deadKey());
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), upperUnicodes.at(i));
    }

    //test for shift off status
    subject->switchLevel(0);
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerUnicodes.at(i));
    }

    // Lock deadkey again.
    subject->click(key);
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

void Ut_MImAbstractKeyArea::testSelectedDeadkeys()
{
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("fr.xml"));
    subject = createKeyArea(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                                              false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);

    // Pick two deadkeys to play around with.
    MImAbstractKey *deadkey1 = keyAt(2, 8); // row 3, column 7
    MImAbstractKey *deadkey2 = keyAt(2, 9); // row 3, column 8
    MImAbstractKey *regularKey = keyAt(0, 0); // first key, top left

    QVERIFY(deadkey1 && deadkey1->isDeadKey());
    QVERIFY(deadkey2 && deadkey2->isDeadKey());
    QVERIFY(regularKey && !regularKey->isDeadKey());

    QCOMPARE(deadkey1->state(), MImAbstractKey::Normal);
    QCOMPARE(deadkey2->state(), MImAbstractKey::Normal);
    QCOMPARE(regularKey->state(), MImAbstractKey::Normal);

    // Press dead key down
    subject->click(deadkey1);
    QCOMPARE(deadkey1->state(), MImAbstractKey::Selected);

    // Release it by clicking regular key
    subject->click(regularKey);
    QCOMPARE(deadkey1->state(), MImAbstractKey::Normal);

    // Down again
    subject->click(deadkey1);
    QCOMPARE(deadkey1->state(), MImAbstractKey::Selected);

    // Release it by clicking itself again.
    subject->click(deadkey1);
    QCOMPARE(deadkey1->state(), MImAbstractKey::Normal);

    // Down again
    subject->click(deadkey1);
    QCOMPARE(deadkey1->state(), MImAbstractKey::Selected);

    // Release it by clicking the other dead key.
    subject->click(deadkey2);
    QCOMPARE(deadkey1->state(), MImAbstractKey::Normal);
    QCOMPARE(deadkey2->state(), MImAbstractKey::Selected);
}

void Ut_MImAbstractKeyArea::testTwoDeadInOne_data()
{
    QTest::addColumn<TestOpList>("operations");
    QTest::addColumn<QString>("expectedCharacterLabel");

    QTest::newRow("no dead key")
        << TestOpList() << "e";
    QTest::newRow("dead key clicked")
        << (TestOpList() << TestOpClickDeadKey) << QString(L'é');
    QTest::newRow("click dead key, shift on")
        << (TestOpList() << TestOpClickDeadKey << TestOpSetShiftOn) << QString(L'É');
    QTest::newRow("shift on, click dead key")
        << (TestOpList() << TestOpSetShiftOn << TestOpClickDeadKey) << QString(L'Ë');
    QTest::newRow("shift on, click dead key, shift off")
        << (TestOpList() << TestOpSetShiftOn << TestOpClickDeadKey << TestOpSetShiftOff) << QString(L'ë');
}

void Ut_MImAbstractKeyArea::testTwoDeadInOne()
{
    QFETCH(TestOpList, operations);
    QFETCH(QString, expectedCharacterLabel);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("test-layout.xml"));
    subject = createKeyArea(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                                              false, 0);

    MImAbstractKey *deadkey = keyAt(2, 8); // accents ´ and ¨
    MImAbstractKey *characterKey = keyAt(0, 2); // e, éë, ÉË

    QVERIFY(deadkey);
    QVERIFY(deadkey->model().binding(false)->isDead());
    QVERIFY(deadkey->model().binding(true)->isDead());

    foreach (TestOperation op, operations) {
        switch (op) {
        case TestOpClickDeadKey:
            subject->click(deadkey);
            break;
        case TestOpSetShiftOn:
            subject->switchLevel(1);
            break;
        case TestOpSetShiftOff:
            subject->switchLevel(0);
            break;
        default:
            QFAIL("Unsupported operation");
            break;
        }
    }

    QCOMPARE(characterKey->label(), expectedCharacterLabel);
}

void Ut_MImAbstractKeyArea::testExtendedLabels()
{
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("test-layout.xml"));
    subject = createKeyArea(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                                              false, 0);

    const MImAbstractKey *eKey(keyAt(0, 2)); // e, ...
    QCOMPARE(eKey->model().binding(false)->extendedLabels(), QString("%1%2").arg(QChar(0xea)).arg(QChar(0xe8)));
    QCOMPARE(eKey->model().binding(true)->extendedLabels(), QString("%1%2").arg(QChar(0xca)).arg(QChar(0xc8)));
}

void Ut_MImAbstractKeyArea::testImportedLayouts_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createKeyArea;
}

void Ut_MImAbstractKeyArea::testImportedLayouts()
{
    QFETCH(KBACreator, createKba);

    // This test uses files test-importer.xml, test-import1.xml, and test-import2.xml.
    // The first imported file test-import1.xml defines some landscape and portrait stuff, while
    // the second imported file test-import2.xml redefines the portrait stuff.

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("test-importer.xml"));
    const LayoutData *model = keyboard->layout(LayoutData::General, M::Landscape);
    QVERIFY(model);
    subject = createKba(model->section(LayoutData::mainSection),
                        false, 0);

    // Check that language and title information are not overwritten by imported files.
    QCOMPARE(keyboard->title(), QString("test-importer-title"));
    QCOMPARE(keyboard->language(), QString("test-importer-language"));

    // First imported defines landscape layout with key labeled "1".
    // Second one doesn't define landscape layout.
    changeOrientation(M::Angle0);
    QCOMPARE(keyAt(0, 0)->label(), QString("1"));
    QCOMPARE(model->section(LayoutData::mainSection)->keyModel(1, 0)->binding(false)->label(),
             QString("func1"));

    // Second imported defines portrait layout with key labeled "2"
    delete subject;
    model = keyboard->layout(LayoutData::General, M::Portrait);
    subject = createKba(model->section(LayoutData::mainSection),
                        false, 0);
    changeOrientation(M::Angle90);
    QCOMPARE(keyAt(0, 0)->label(), QString("2"));
    QCOMPARE(model->section(LayoutData::mainSection)->keyModel(1, 0)->binding(false)->label(),
             QString("func2"));
}

void Ut_MImAbstractKeyArea::testPopup_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createKeyArea;
}

void Ut_MImAbstractKeyArea::testPopup()
{
    TpCreator createTp = &MImAbstractKeyArea::createTouchPoint;
    QFETCH(KBACreator, createKba);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));
    subject = createKba(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        true, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    const QPoint mousePos(subject->style()->paddingTop() + 1, subject->style()->paddingLeft() + 1); // approximately the top left key on layout
    QVERIFY(&subject->popup());

    QTouchEvent::TouchPoint tp(0);
    tp.setScreenPos(mousePos);
    subject->touchPointPressed(createTp(0, Qt::TouchPointPressed,
                                        subject->mapToScene(mousePos),
                                        QPointF()));

    QVERIFY(subject->popup().isVisible());
    subject->touchPointReleased(createTp(0, Qt::TouchPointReleased,
                                         subject->mapToScene(mousePos),
                                         subject->mapToScene(mousePos)));
}

void Ut_MImAbstractKeyArea::testInitialization_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createKeyArea;
}

void Ut_MImAbstractKeyArea::testInitialization()
{
    QFETCH(KBACreator, createKba);
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));
    subject = createKba(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        false, 0);
    subject->resize(defaultLayoutSize());

}

void Ut_MImAbstractKeyArea::testShiftCapsLock()
{
    // Load any layout that has shift
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));
    const LayoutData *layout = keyboard->layout(LayoutData::General, M::Landscape);
    QVERIFY(layout);
    const LayoutData::SharedLayoutSection section = layout->section(LayoutData::mainSection);

    subject = createKeyArea(section,
                                              false, 0);

    MImKey *shiftButton = static_cast<MImKeyArea *>(subject)->shiftKey;
    QVERIFY(shiftButton);
    QVERIFY(shiftButton->state() == MImAbstractKey::Normal);

    subject->setShiftState(ModifierLockedState);
    QVERIFY(shiftButton->state() == MImAbstractKey::Selected);

    subject->setShiftState(ModifierClearState);
    QVERIFY(shiftButton->state() == MImAbstractKey::Normal);
}

void Ut_MImAbstractKeyArea::testMultiTouch()
{
    QSKIP("Move signal tests into separate slot, and remove this one (overlapping tests)",
          SkipAll);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));
    const LayoutData *layout = keyboard->layout(LayoutData::General, M::Landscape);
    QVERIFY(layout);
    const LayoutData::SharedLayoutSection section = layout->section(LayoutData::mainSection);

    subject = createKeyArea(section,
                                              false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    const MImAbstractKey *key0 = keyAt(0, 0);
    const MImAbstractKey *key1 = keyAt(1, 0);
    const MImAbstractKey *key2 = keyAt(0, 1);

    QVERIFY(key0);
    QVERIFY(key1);
    QVERIFY(key2);

    QSignalSpy pressed(subject, SIGNAL(keyPressed(const MImAbstractKey*, const QString&, bool)));
    QSignalSpy released(subject, SIGNAL(keyReleased(const MImAbstractKey*, const QString&, bool)));
    QSignalSpy clicked(subject, SIGNAL(keyClicked(const MImAbstractKey*, const QString&, bool, const QPoint&)));

    QVERIFY(pressed.isValid());
    QVERIFY(released.isValid());
    QVERIFY(clicked.isValid());

    const QPoint pos0 = key0->buttonRect().center().toPoint();
    QTouchEvent::TouchPoint tp0(0);
    tp0.setScreenPos(pos0);

    const QPoint pos1 = key1->buttonRect().center().toPoint();
    QTouchEvent::TouchPoint tp1(1);
    tp1.setScreenPos(pos1);

    const QPoint pos2 = key2->buttonRect().center().toPoint();
    QTouchEvent::TouchPoint tp2(2);
    tp2.setScreenPos(pos2);

    /*
     * Verify following conditions:
     * 1) signals are emitted in correct order
     * 2) every signal corresponds to correct key
     */
    subject->touchPointPressed(tp0);
    QCOMPARE(pressed.count(), 1);
    QVERIFY(pressed.at(0).first().value<const MImAbstractKey*>() == key0);
    QCOMPARE(released.count(), 0);
    QCOMPARE(clicked.count(), 0);

    subject->touchPointPressed(tp1);
    QCOMPARE(pressed.count(), 2);
    QVERIFY(pressed.at(1).first().value<const MImAbstractKey*>() == key1);
    QCOMPARE(released.count(), 0);
    QCOMPARE(clicked.count(), 0);

    subject->touchPointReleased(tp0);
    subject->touchPointReleased(tp1);
    QCOMPARE(pressed.count(), 2);
    QCOMPARE(released.count(), 2);
    QVERIFY(released.at(0).first().value<const MImAbstractKey*>() == key0);
    QVERIFY(released.at(1).first().value<const MImAbstractKey*>() == key1);
    QCOMPARE(clicked.count(), 2);
    QVERIFY(clicked.at(0).first().value<const MImAbstractKey*>() == key0);
    QVERIFY(clicked.at(1).first().value<const MImAbstractKey*>() == key1);

    pressed.clear();
    released.clear();
    clicked.clear();

    // Verify if could click on some keys while other key is pressed
    subject->touchPointPressed(tp0);
    subject->touchPointPressed(tp1);
    subject->touchPointReleased(tp0);
    subject->touchPointPressed(tp2);
    subject->touchPointReleased(tp2);
    subject->touchPointReleased(tp1);

    QCOMPARE(pressed.count(), 3);
    QCOMPARE(released.count(), 3);
    QCOMPARE(clicked.count(), 3);

    QVERIFY(pressed.at(0).first().value<const MImAbstractKey*>() == key0);
    QVERIFY(pressed.at(1).first().value<const MImAbstractKey*>() == key1);
    QVERIFY(pressed.at(2).first().value<const MImAbstractKey*>() == key2);

    QVERIFY(released.at(0).first().value<const MImAbstractKey*>() == key0);
    QVERIFY(released.at(1).first().value<const MImAbstractKey*>() == key2);
    QVERIFY(released.at(2).first().value<const MImAbstractKey*>() == key1);

    QVERIFY(clicked.at(0).first().value<const MImAbstractKey*>() == key0);
    QVERIFY(clicked.at(1).first().value<const MImAbstractKey*>() == key2);
    QVERIFY(clicked.at(2).first().value<const MImAbstractKey*>() == key1);
}

void Ut_MImAbstractKeyArea::testRtlKeys_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::addColumn<M::Orientation>("orientation");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QList<MImKeyBinding::KeyAction> >("expectedRtlKeys");

    const QString ar("ar.xml");
    const QString en_gb("en_gb.xml");
    QList<MImKeyBinding::KeyAction> rtlKeys;
    const QList<MImKeyBinding::KeyAction> nothing;

    rtlKeys << MImKeyBinding::ActionBackspace;

    QTest::newRow("SingleWidgetArea Landscape Arabic")
        << &createKeyArea
        << M::Landscape
        << ar
        << rtlKeys;

    QTest::newRow("SingleWidgetArea Portrait Arabic" )
        << &createKeyArea
        << M::Portrait
        << ar
        << rtlKeys;

    QTest::newRow("SingleWidgetArea Landscape English")
        << &createKeyArea
        << M::Landscape
        << en_gb
        << nothing;

    QTest::newRow("SingleWidgetArea Portrait English" )
        << &createKeyArea
        << M::Portrait
        << en_gb
        << nothing;
}

void Ut_MImAbstractKeyArea::testRtlKeys()
{
    QFETCH(KBACreator, createKba);
    QFETCH(M::Orientation, orientation);
    QFETCH(QString, fileName);
    QFETCH(QList<MImKeyBinding::KeyAction>, expectedRtlKeys);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(fileName));
    subject = createKba(keyboard->layout(LayoutData::General, orientation)->section(LayoutData::mainSection),
                        false, 0);

    MImKeyArea *keyArea = dynamic_cast<MImKeyArea *>(subject);

    QVERIFY2(keyArea != 0, "Unknown type of button area");
    for (int row = 0; row < keyArea->rowCount(); ++row) {
        for (int column = 0; column < keyArea->sectionModel()->columnsAt(row); ++column) {
            MImKey *key = keyArea->rowList[row].keys[column];
            QVERIFY(key != 0);
            if (expectedRtlKeys.contains(key->model().binding()->action())) {
                QVERIFY(key->model().rtl());
                QVERIFY2(key->iconId().contains("-rtl-"), "This is not RTL icon");
            } else if (!key->iconId().isEmpty()) {
                QVERIFY(!key->model().rtl());
                QVERIFY2(!key->iconId().contains("-rtl-"), "This is not LTR icon");
            }
        }
    }
}

void Ut_MImAbstractKeyArea::testLongKeyPress()
{
    TpCreator createTp = &MImAbstractKeyArea::createTouchPoint;
    const int LongPressTimeOut = 1500; //this value depends on timeout of long press

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));

    subject = createKeyArea(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                                              true, 0);

    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    MImKey *key0 = dynamic_cast<MImKey *>(keyAt(0, 0));
    MImKey *key1 = dynamic_cast<MImKey *>(keyAt(1, 3));

    QVERIFY(key0);
    QVERIFY(key1);

    QPoint point0 = key0->buttonBoundingRect().center().toPoint();
    QTouchEvent::TouchPoint tp0(createTp(0, Qt::TouchPointPressed,
                                         subject->mapToScene(point0),
                                         QPointF()));

    QPoint point1 = key1->buttonBoundingRect().center().toPoint();
    QTouchEvent::TouchPoint tp1(createTp(1, Qt::TouchPointPressed,
                                         subject->mapToScene(point1),
                                         QPointF()));

    QSignalSpy spy(subject, SIGNAL(longKeyPressed(const MImAbstractKey*, const QString &, bool)));

    QVERIFY(spy.isValid());

    // click is not long press
    subject->touchPointPressed(tp0);
    subject->touchPointReleased(tp1);
    QVERIFY(spy.isEmpty());
    QTest::qWait(LongPressTimeOut);
    QVERIFY(spy.isEmpty());

    // long press on the key
    subject->touchPointPressed(tp0);
    QTest::qWait(LongPressTimeOut);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().count() > 0);
    QCOMPARE(spy.first().first().value<const MImAbstractKey *>(), key0);
    spy.clear();
    subject->touchPointReleased(tp0);

    // long press with multitouch
    subject->touchPointPressed(tp0);
    subject->touchPointPressed(tp1);
    QTest::qWait(LongPressTimeOut);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().count() > 0);
    QCOMPARE(spy.first().first().value<const MImAbstractKey *>(), key1);
    spy.clear();
    subject->touchPointReleased(tp0);
    subject->touchPointReleased(tp1);
    QVERIFY(spy.isEmpty());

    // long press after movement
    subject->touchPointPressed(tp0);
    tp0.setStartScenePos(QPointF(52, 37));
    subject->touchPointMoved(tp0);
    QTest::qWait(LongPressTimeOut);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().count() > 0);
    QCOMPARE(spy.first().first().value<const MImAbstractKey *>(), key0);
    spy.clear();
    subject->touchPointReleased(tp0);
    QVERIFY(spy.isEmpty());

    // TODO? long press should be detected if last pressed key is not released
}

void Ut_MImAbstractKeyArea::testKeyLayout()
{
    const int margin = 5;
    const QSize size(50, 50);
    subject = Ut_MImAbstractKeyArea::createArea("Q", size, QSize(size.width() - 2 * margin,
                                                                 size.height() - 2 * margin));

    MImAbstractKeyAreaStyle *s = const_cast<MImAbstractKeyAreaStyle *>(subject->style().operator->());
    s->setPaddingLeft(margin);
    s->setPaddingRight(margin);
    s->setPaddingTop(margin);
    s->setPaddingBottom(margin);

    subject->updateKeyGeometries(size.width());

    QVERIFY(!subject->keyAt(QPoint(-1, -1)));
    QVERIFY(subject->keyAt(QPoint(0, 0)));
    QVERIFY(subject->keyAt(QPoint(size.width() / 2, size.height() / 2)));
    QVERIFY(subject->keyAt(QPoint(size.width(), size.height())));
    QVERIFY(!subject->keyAt(QPoint(size.width() + 1, size.height() + 1)));
}

void Ut_MImAbstractKeyArea::testTouchPoints_data()
{
    TpCreator createTp = &MImAbstractKeyArea::createTouchPoint;

    QTest::addColumn<int>("expectedClickedSignals");
    QTest::addColumn<QString>("labels");
    QTest::addColumn<QSize>("kbaSize");

    QTest::addColumn<TpList>("touchPoints");
    QTest::addColumn<TpButtonStateMatrix>("expectedStates");

    QTest::newRow("single button")
        << 1 << "a" << QSize(50, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(12, 24), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointReleased, QPointF(16, 20), QPointF(12, 24)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal));

    QTest::newRow("single button, commit before next hit")
        << 2 << "a" << QSize(50, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(12, 24), QPointF(-1, -1))
                     << createTp(1, Qt::TouchPointPressed, QPointF(16, 20), QPointF(-1, -1))
                     << createTp(1, Qt::TouchPointReleased, QPointF(16, 20), QPointF(12, 24))
                     << createTp(0, Qt::TouchPointReleased, QPointF(16, 20), QPointF(16, 20)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal));

    QTest::newRow("2 buttons, 3 touchpoint transactions")
        << 3 << "ab" << QSize(100, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(12, 24), QPointF(-1, -1))
                     << createTp(1, Qt::TouchPointPressed, QPointF(80, 20), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointReleased, QPointF(16, 20), QPointF(12, 24))
                     << createTp(1, Qt::TouchPointPressed, QPointF(12, 24), QPointF(-1, -1))
                     << createTp(1, Qt::TouchPointReleased, QPointF(16, 20), QPointF(16, 24))
                     << createTp(0, Qt::TouchPointReleased, QPointF(80, 20), QPointF(80, 20)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Normal));

    QTest::newRow("move into button, release")
        << 1 << "a" << QSize(50, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(-1, -1), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(2, -1), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(10, 10), QPointF(2, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(12, 14), QPointF(10, 10))
                     << createTp(0, Qt::TouchPointReleased, QPointF(12, 14), QPointF(12, 14)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal));

    QTest::newRow("move over button, release outside")
        << 0 << "a" << QSize(50, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(-1, -1), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(2, -1), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(10, 10), QPointF(2, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(51, 51), QPointF(10, 10))
                     << createTp(0, Qt::TouchPointReleased, QPointF(51, 51), QPointF(51, 51)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal));

    QTest::newRow("tap inside button, release outside")
        << 0 << "a" << QSize(50, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(12, 12), QPointF(12, 12))
                     << createTp(0, Qt::TouchPointMoved, QPointF(2, -1), QPointF(12, 12))
                     << createTp(0, Qt::TouchPointMoved, QPointF(10, 10), QPointF(2, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(30, 30), QPointF(10, 10))
                     << createTp(0, Qt::TouchPointReleased, QPointF(51, 51), QPointF(30, 30)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal));

    QTest::newRow("move into button, press on button with second touchpoint, release first")
        << 2 << "a" << QSize(50, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(-1, -1), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(2, -1), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(10, 10), QPointF(2, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(12, 14), QPointF(10, 10))
                     << createTp(1, Qt::TouchPointPressed, QPointF(20, 20), QPointF(20, 20))
                     << createTp(0, Qt::TouchPointReleased, QPointF(12, 14), QPointF(12, 14))
                     << createTp(0, Qt::TouchPointMoved, QPointF(51, 51), QPointF(20, 20)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal));

    QTest::newRow("sudden move from a to b")
        << 1 << "ab" << QSize(100, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(12, 24), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(30, 20), QPointF(12, 24))
                     << createTp(0, Qt::TouchPointMoved, QPointF(80, 20), QPointF(30, 20))
                     << createTp(0, Qt::TouchPointReleased, QPointF(80, 20), QPointF(80, 20)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Normal));

    QTest::newRow("sudden release while moving from a to b")
        << 0 << "ab" << QSize(100, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(12, 24), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(30, 20), QPointF(12, 24))
                     << createTp(0, Qt::TouchPointReleased, QPointF(80, 20), QPointF(30, 20)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Normal));

    QTest::newRow("sudden release while moving from a to b, mixed with other touchpoint transaction")
        << 2 << "ab" << QSize(100, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(12, 24), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(30, 20), QPointF(12, 24))
                     << createTp(1, Qt::TouchPointPressed, QPointF(70, 16), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointReleased, QPointF(80, 20), QPointF(30, 20))
                     << createTp(1, Qt::TouchPointMoved, QPointF(-1, -1), QPointF(70, 16)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Normal));
}

void Ut_MImAbstractKeyArea::testTouchPoints()
{
    // How it works:
    // For each touchpoint, I send it to the appropiate handler in KBA.
    // I then compare the button state against the expected button state.
    // It might look a bit complex, but actually it isn't.
    // touchPoints is just a series of synthetic Qt touchpoints.
    // The expectedStates maxtrix has strlen(labels) rows and
    // count(touchPoints) columns.

    QFETCH(int, expectedClickedSignals);
    QFETCH(QString, labels);
    QFETCH(QSize, kbaSize);
    QFETCH(TpList, touchPoints);
    QFETCH(TpButtonStateMatrix, expectedStates);

    subject = Ut_MImAbstractKeyArea::createArea(labels, kbaSize);
    QSignalSpy spy(subject, SIGNAL(keyClicked(const MImAbstractKey*, QString, bool, QPoint)));

    ButtonList tracedButtons;
    for (int i = 0; i < labels.count(); ++i) {
        tracedButtons << keyAt(0, i);
    }

    for (int i = 0; i < touchPoints.size(); ++i) {
        QTouchEvent::TouchPoint tp = touchPoints.at(i);

        switch (tp.state()) {
        case Qt::TouchPointPressed:
            subject->touchPointPressed(tp);
            break;

        case Qt::TouchPointMoved:
            subject->touchPointMoved(tp);
            break;

        case Qt::TouchPointReleased:
            subject->touchPointReleased(tp);
            break;

        default:
            break;
        }

        const ButtonStateList &bsl = expectedStates.at(i);
        for (int k = 0; k < bsl.size(); ++k) {
            QCOMPARE(tracedButtons.at(k)->state(), bsl.at(k));
        }
    }

    QCOMPARE(spy.count(), expectedClickedSignals);
}

void Ut_MImAbstractKeyArea::changeOrientation(M::OrientationAngle angle)
{
    if (MPlainWindow::instance()->orientationAngle() != angle) {
        MPlainWindow::instance()->setOrientationAngle(angle);
        //timeout depends on duration of orientation animation
        QTest::qWait(1000);
    }
}

QSize Ut_MImAbstractKeyArea::defaultLayoutSize()
{
    // Take visible scene size as layout size, but reduce keyboard's paddings first from its width.
    // The height value is ignored since MImAbstractKeyAreas determine their own height.
    return MPlainWindow::instance()->visibleSceneSize()
            - QSize(subject->style()->paddingLeft() + subject->style()->paddingRight(), 0);
}

// Helper method to get key in certain row and column from current subject.
MImAbstractKey *Ut_MImAbstractKeyArea::keyAt(unsigned int row, unsigned int column) const
{
    // If this fails there is something wrong with the test.
    Q_ASSERT(subject
             && (row < static_cast<unsigned int>(subject->rowCount()))
             && (column < static_cast<unsigned int>(subject->sectionModel()->columnsAt(row))));

    MImAbstractKey *key = 0;

    MImKeyArea *buttonArea = dynamic_cast<MImKeyArea *>(subject);
    if (buttonArea) {
        key = buttonArea->rowList[row].keys[column];
    }

    return key;
}

MImAbstractKeyArea *Ut_MImAbstractKeyArea::createArea(const QString &labels,
                                                      const QSize &size,
                                                      const QSize &fixedNormalKeySize)
{
    LayoutData::SharedLayoutSection section;
    section = LayoutData::SharedLayoutSection(new LayoutSection(labels));
    MImKeyArea *swba = new MImKeyArea(LayoutData::SharedLayoutSection(section));

    // Reset the style:
    MImAbstractKeyAreaStyle *s = const_cast<MImAbstractKeyAreaStyle *>(swba->style().operator->());

    // Margins:
    s->setMarginLeft(0);
    s->setMarginTop(0);
    s->setMarginRight(0);
    s->setMarginBottom(0);

    // Paddings:
    s->setPaddingLeft(1);
    s->setPaddingTop(1);
    s->setPaddingRight(1);
    s->setPaddingBottom(1);

    // Key geometry:
    s->setKeyHeightMedium(fixedNormalKeySize.height());
    s->setKeyWidthMediumFixed(fixedNormalKeySize.width());
    s->setKeyWidthMedium(1.0);

    // Behaviour:
    s->setTouchpointHorizontalGravity(0);
    s->setTouchpointVerticalGravity(0);

    swba->resize(size);

    return swba;
}


QTEST_APPLESS_MAIN(Ut_MImAbstractKeyArea);
