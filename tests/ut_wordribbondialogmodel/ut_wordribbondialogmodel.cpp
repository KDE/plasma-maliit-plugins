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



#include "ut_wordribbondialogmodel.h"
#include "wordribbondialogmodel.h"
#include "mwidget.h"
#include "mplainwindow.h"
#include "utils.h"
#include <MTheme>
#include <MList>
#include <QtTest/QTest>
#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QStringListModel>
#include <QSignalSpy>
#include <QGraphicsSceneMouseEvent>
#include <MSceneManager>

namespace{
    int LandscapeWidth = 864;
    int protraitWidth = 480;
}

void Ut_WordRibbonDialogModel::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_mimcorrectioncandidatewidget",
                                  (char *) "-local-theme" };
    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);
    view = new MPlainWindow();

    subject = 0;
}

void Ut_WordRibbonDialogModel::cleanupTestCase()
{
    delete view;
    view = 0;
    delete app;
    app = NULL;
}

void Ut_WordRibbonDialogModel::init()
{
    subject = new WordRibbonDialogModel();
    QVERIFY(subject != 0);
}


void Ut_WordRibbonDialogModel::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_WordRibbonDialogModel::testRowCount()
{
    QStringList list;
    list.clear();
    subject->setCandidates(list, -1);
    QVERIFY(subject->rowCount() == 0);
    subject->setCandidates(list, 0);
    QVERIFY(subject->rowCount() == 0);
    subject->setCandidates(list, LandscapeWidth);
    QVERIFY(subject->rowCount() == 0);
    subject->setCandidates(list, protraitWidth);
    QVERIFY(subject->rowCount() == 0);

    list.clear();
    list << "a";
    subject->setCandidates(list, -1);
    QVERIFY(subject->rowCount() == 0);
    subject->setCandidates(list, 0);
    QVERIFY(subject->rowCount() == 0);
    subject->setCandidates(list, LandscapeWidth);
    QVERIFY(subject->rowCount() == 1);
    subject->setCandidates(list, protraitWidth);
    QVERIFY(subject->rowCount() == 1);

    list.clear();
    list << "a" << "b" << "c" << "d" << "e" << "f" << "g" << "h" << "i"
          << "j" << "k" << "l" << "m" << "n" << "o" << "p" << "q" << "r";
    subject->setCandidates(list, -1);
    QVERIFY(subject->rowCount() == 0);
    subject->setCandidates(list, 0);
    QVERIFY(subject->rowCount() == 0);
    subject->setCandidates(list, LandscapeWidth);
    QVERIFY(subject->rowCount() > 0);
    QVERIFY(subject->rowCount() <= list.count());
    subject->setCandidates(list, protraitWidth);
    QVERIFY(subject->rowCount() > 0);
    QVERIFY(subject->rowCount() <= list.count());

    list.clear();
    list << "aaa" << "bbb" << "ccc" << "d" << "e" << "f" << "g" << "h" << "i"
          << "j" << "k" << "llll" << "mmm" << "n" << "o" << "pp" << "q" << "r";
    subject->setCandidates(list, -1);
    QVERIFY(subject->rowCount() == 0);
    subject->setCandidates(list, 0);
    QVERIFY(subject->rowCount() == 0);
    subject->setCandidates(list, LandscapeWidth);
    QVERIFY(subject->rowCount() > 0);
    QVERIFY(subject->rowCount() <= list.count());
    subject->setCandidates(list, protraitWidth);
    QVERIFY(subject->rowCount() > 0);
    QVERIFY(subject->rowCount() <= list.count());
}

QTEST_APPLESS_MAIN(Ut_WordRibbonDialogModel);


