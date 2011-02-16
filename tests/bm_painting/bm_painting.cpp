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


#include "bm_painting.h"
#include "paintrunner.h"
#include "loggingwindow.h"
#include "mimkeyarea.h"
#include "keyboarddata.h"
#include "utils.h"
#include "horizontalswitcher.h"

// this test could not be compiled for Windows
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <MApplication>
#include <MTheme>
#include <MSceneWindow>
#include <MScene>
#include <MSceneManager>

#include <QDir>
#include <QtGlobal>

namespace {
    int gArgc = 2;
    char *gArgv[2] = { (char *) "bm_painting",
                       (char *) "-software" };
    const char *const MImUserDirectory = ".meego-im";
    const int DefaultDelay = 1000; // milliseconds
}

void Bm_Painting::initTestCase()
{
    disableQtPlugins();

    QDir dir;
    dir.mkpath(QString("%1/%2").arg(QDir::homePath()).arg(MImUserDirectory));
}

void Bm_Painting::cleanupTestCase()
{
}

void Bm_Painting::init()
{
    sceneWindow = 0;
    window = 0;
    widget = 0;
    app = 0;
}

void Bm_Painting::cleanup()
{
    delete sceneWindow;
    sceneWindow = 0;
    delete window;
    window = 0;
    delete widget;
    widget = 0;
    delete app;
    app = 0;
}

/*
 * COMPOSITE variable defines whether main window should be
 * composited or not. Possible values (case sensitive):
 *     * <unset> - perform tests for both opaque and tranclucent windows
 *     * true - perform tests for tranclucent (composited) window
 *     * any other - perform tests for opaque window
 * HARDWARE variable defines whether hardware acceleration should be used or
 * not. Possible values:
 *     * <unset> - perform tests with and without hardware acceleration
 *     * true - perform tests with hardware acceleration
 *     * any other - perform tests without hardware acceleration
 * DELAY defines duration of one step in test case. Default value is DefaultDelay
 */
void Bm_Painting::commonDataSetup(const QString& testcaseName)
{
    QDir dir("/usr/share/meegotouch/virtual-keyboard/layouts/");
    QStringList filters;
    QFileInfoList files;
    QFileInfo info;
    QString resultFilenames[2][2];
    QString fileNameTemplate(QString("%1/%2/%3-%4-%5").arg(QDir::homePath())
                                                   .arg(MImUserDirectory)
                                                   .arg(QCoreApplication::applicationPid())
                                                   .arg(testcaseName));
    int hwMin = 0;
    int hwMax = 1;
    int compositeMin = 0;
    int compositeMax = 1;
    int delay = DefaultDelay;
    QString env;

    env = qgetenv("COMPOSITE");
    if (!env.isEmpty()) {
        compositeMin = compositeMax = ((env == "true") ? 1 : 0);
    }

    env = qgetenv("HARDWARE");
    if (!env.isEmpty()) {
        hwMin = hwMax = ((env == "true") ? 1 : 0);
    }

    env = qgetenv("DELAY");
    if (!env.isEmpty()) {
        delay = env.toInt();
    }

    QTest::addColumn<QString>("filename");
    QTest::addColumn<bool>("hardwareRendering");
    QTest::addColumn<bool>("compositing");
    QTest::addColumn<QString>("resultFilename");
    QTest::addColumn<int>("delay");

    resultFilenames[0][0] = fileNameTemplate.arg("sw_opaque.csv");
    resultFilenames[1][0] = fileNameTemplate.arg("hw_opaque.csv");
    resultFilenames[0][1] = fileNameTemplate.arg("sw_composite.csv");
    resultFilenames[1][1] = fileNameTemplate.arg("hw_composite.csv");

    filters << "en_gb.xml";
    files = dir.entryInfoList(filters);

    for (int composite = compositeMin; composite <= compositeMax; ++composite) {
        for (int hw = hwMin; hw <= hwMax; ++hw) {
            for (int n = files.count() - 1; n >= 0; --n) {
                info = files.at(n);
                QString caseName = QString("%1: file=%2 composite=%3 hw=%4 results=%5").arg(testcaseName).
                                arg(info.fileName()).arg(composite).arg(hw).
                                arg(resultFilenames[hw][composite]);
                QTest::newRow(caseName.toLatin1().constData()) << info.fileName()
                                                               << bool(hw)
                                                               << bool(composite)
                                                               << resultFilenames[hw][composite]
                                                               << delay;
            }
        }
    }
}

// Sets up widget, window, app, sceneWindow
// Cleanup handled by ::cleanup()
void Bm_Painting::commonWindowSetup(bool hardwareRendering, bool compositing)
{
    gArgc = hardwareRendering ? 1 : 2;

    app = new MApplication(gArgc, gArgv);

    widget = new QWidget;
    if (compositing) {
        widget->setAttribute(Qt::WA_TranslucentBackground);
    }
    Qt::WindowFlags windowFlags = Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;
    widget->setWindowFlags(windowFlags);
    widget->show();

    window = new LoggingWindow(widget);
    window->setTranslucentBackground(!MApplication::softwareRendering());
    if (MApplication::softwareRendering())
        window->viewport()->setAutoFillBackground(false);
    sceneWindow = createMSceneWindow(window);
    window->show();
    QTest::qWaitForWindowShown(window);

    QSize sceneSize = window->visibleSceneSize(M::Landscape);
    int w = sceneSize.width();
    int h = sceneSize.height();
    window->setSceneRect(0, 0, w, h);

    widget->resize(sceneSize);
}

void Bm_Painting::benchmarkPaintDuringKeyPresses_data()
{
    commonDataSetup("benchmarkPaintDuringKeyPresses");
}

void Bm_Painting::benchmarkPaintDuringKeyPresses()
{
    QFETCH(QString, filename);
    QFETCH(bool, hardwareRendering);
    QFETCH(bool, compositing);
    QFETCH(QString, resultFilename);
    QFETCH(int, delay);

    // Setup
    commonWindowSetup(hardwareRendering, compositing);

    KeyboardData *keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(filename));
    MImKeyArea *subject = new MImKeyArea(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection));

    subject->resize(defaultLayoutSize(subject));

    PaintRunner runner;
    window->scene()->addItem(&runner);
    runner.update();

    subject->setParentItem(&runner);

    MImKey *key0 = dynamic_cast<MImKey *>(keyAt(subject, 0, 0));
    MImKey *key1 = dynamic_cast<MImKey *>(keyAt(subject, 1, 3));
    MImKey *key2 = dynamic_cast<MImKey *>(keyAt(subject, 2, 6));

    QVERIFY(key0);
    QVERIFY(key1);
    QVERIFY(key2);

    const QPoint point0 = key0->buttonBoundingRect().center().toPoint();
    const QPoint point1 = key1->buttonBoundingRect().center().toPoint();
    const QPoint point2 = key2->buttonBoundingRect().center().toPoint();

    QTouchEvent::TouchPoint press0(MImAbstractKeyArea::createTouchPoint(0, Qt::TouchPointPressed,
                                                                        subject->mapToScene(point0),
                                                                        QPointF()));
    QTouchEvent::TouchPoint release0(MImAbstractKeyArea::createTouchPoint(0, Qt::TouchPointReleased,
                                                                          subject->mapToScene(point0),
                                                                          QPointF()));

    QTouchEvent::TouchPoint press1(MImAbstractKeyArea::createTouchPoint(1, Qt::TouchPointPressed,
                                                                        subject->mapToScene(point1),
                                                                        QPointF()));
    QTouchEvent::TouchPoint release1(MImAbstractKeyArea::createTouchPoint(1, Qt::TouchPointReleased,
                                                                          subject->mapToScene(point1),
                                                                          QPointF()));

    QTouchEvent::TouchPoint press2(MImAbstractKeyArea::createTouchPoint(2, Qt::TouchPointPressed,
                                                                        subject->mapToScene(point2),
                                                                        QPointF()));
    QTouchEvent::TouchPoint release2(MImAbstractKeyArea::createTouchPoint(2, Qt::TouchPointReleased,
                                                                          subject->mapToScene(point2),
                                                                          QPointF()));

    QList< QList<QTouchEvent::TouchPoint> > plannedEvents;
    QList<QTouchEvent::TouchPoint> eventList;

    plannedEvents << eventList;

    eventList << press0;
    plannedEvents << eventList;
    eventList.clear();

    eventList << release0;
    plannedEvents << eventList;
    eventList.clear();

    eventList << press0 << press1;
    plannedEvents << eventList;
    eventList.clear();

    eventList << release0 << release1;
    plannedEvents << eventList;
    eventList.clear();

    eventList << press0 << press1 << press2;
    plannedEvents << eventList;
    eventList.clear();

    eventList << release0 << release1 << release2;
    plannedEvents << eventList;
    eventList.clear();

    // Execute test
    window->loggingEnabled = true;
    foreach (eventList, plannedEvents) {
        foreach (const QTouchEvent::TouchPoint &event, eventList) {
            if (event.state() == Qt::TouchPointPressed) {
                qDebug() << "press " << event.scenePos();
                subject->touchPointPressed(event);
            } else if (event.state() == Qt::TouchPointReleased) {
                qDebug() << "release " << event.scenePos();
                subject->touchPointReleased(event);
            }
        }
        qDebug() << "***";
        window->logMark();
        QTest::qWait(delay);
    }
    window->loggingEnabled = false;

    // Cleanup
    window->writeResults(resultFilename);

    subject->setParentItem(0);
    delete subject;
    delete keyboard;
}

void Bm_Painting::benchmarkPaintDuringHorizontalLayoutChange_data()
{
    commonDataSetup("benchmarkPaintDuringHorizontalLayoutChange");
}

void Bm_Painting::benchmarkPaintDuringHorizontalLayoutChange()
{
    QFETCH(QString, filename);
    QFETCH(bool, hardwareRendering);
    QFETCH(bool, compositing);
    QFETCH(QString, resultFilename);
    QFETCH(int, delay);

    HorizontalSwitcher::SwitchDirection direction = HorizontalSwitcher::Left;
    int numberOfLayoutChanges = 5;

    // Setup
    commonWindowSetup(hardwareRendering, compositing);

    KeyboardData *keyboardData = new KeyboardData;
    QVERIFY(keyboardData->loadNokiaKeyboard(filename));
    const LayoutData::SharedLayoutSection &section = keyboardData->
            layout(LayoutData::General, M::Landscape)->
            section(LayoutData::mainSection);

    HorizontalSwitcher *subject = new HorizontalSwitcher();
    MImKeyArea *keyArea = 0;
    int numberOfKeyAreas = 5;
    for(int i=0; i<numberOfKeyAreas; i++) {
        keyArea = new MImKeyArea(section, false, subject); // FIXME: enable popup?
        subject->addWidget(keyArea);
    }
    subject->setLooping(true);
    subject->setCurrent(0);

    subject->resize(defaultLayoutSize(keyArea));
    PaintRunner runner;
    window->scene()->addItem(&runner);
    runner.update();
    subject->setParentItem(&runner);

    // Execute test
    window->loggingEnabled = true;
    for(int i=0; i<numberOfLayoutChanges; i++)
    {
        subject->switchTo(direction);
        QTest::qWait(delay);
    }
    window->loggingEnabled = false;

    // Cleanup
    window->writeResults(resultFilename);

    subject->setParentItem(0);
    delete subject;
    delete keyboardData;
}

QSize Bm_Painting::defaultLayoutSize(MImKeyArea *keyArea)
{
    // Take visible scene size as layout size, but reduce keyboard's paddings first from its width.
    // The height value is ignored since MImAbstractKeyAreas determine their own height.
    return window->visibleSceneSize()
            - QSize(keyArea->style()->paddingLeft() + keyArea->style()->paddingRight(), 0);
}

MImAbstractKey *Bm_Painting::keyAt(MImKeyArea *keyArea, unsigned int row, unsigned int column) const
{
    // If this fails there is something wrong with the test.
    Q_ASSERT(keyArea
             && (row < static_cast<unsigned int>(keyArea->rowCount()))
             && (column < static_cast<unsigned int>(keyArea->sectionModel()->columnsAt(row))));

    return keyArea->rowList[row].keys[column];
}

QTEST_APPLESS_MAIN(Bm_Painting);
