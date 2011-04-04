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



#include "ut_wordribbondialog.h"
#include "wordribbondialog.h"
#include "wordribbondialogmodel.h"
#include "mwidget.h"
#include "mplainwindow.h"
#include "utils.h"
#include "regiontracker.h"

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

#include <QMetaType>
#include <MApplication>
#include <MApplicationWindow>

#include <MWidget>

#include <MButton>
#include <MDialog>
#include <MSceneWindow>
#include <MLayout>

#include <mbuttonmodel.h>



void Ut_WordRibbonDialog::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_wordribbondialog",
                                  (char *) "-local-theme" };
    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);
    view = new MPlainWindow;

}

void Ut_WordRibbonDialog::cleanupTestCase()
{
    delete view;
    view = 0;
    delete app;
    app = NULL;
}

void Ut_WordRibbonDialog::init()
{
    RegionTracker::createInstance();
    appWin = new MWindow;
    QVERIFY(appWin != 0);
    subject = new WordRibbonDialog();
    QVERIFY(subject != 0);

    QApplication::processEvents(QEventLoop::WaitForMoreEvents, 10);
}

void Ut_WordRibbonDialog::cleanup()
{
    delete subject;
    subject = 0;
    delete appWin;
    appWin = 0;
    RegionTracker::destroyInstance();
}

void Ut_WordRibbonDialog::testSetCandidates()
{
    subject->appear(appWin);

    QStringList list;
    list << "aaa" << "bbb" << "ccc" << "d" << "e" << "f" << "g" << "h" << "i"
          << "j" << "k" << "llll" << "mmm" << "n" << "o" << "pp" << "q" << "r";

    subject->setCandidates(list, "test");
    QVERIFY(subject->candidatesList == list);
    QVERIFY(subject->dataModel->rowCount() > 2);
    if (subject->dataModel->rowCount() > 2) {
        QStringList tmpList;
        for (int i = 0; i < subject->dataModel->rowCount(); ++i) {
            tmpList += subject->dataModel->data(subject->dataModel->index(i), Qt::DisplayRole).toStringList();
        }
        QVERIFY(tmpList == list);
    }

    list.clear();
    list << "a";

    subject->setCandidates(list, "test");
    QVERIFY(subject->candidatesList == list);
    QVERIFY(subject->dataModel->rowCount() == 1);
    if (subject->dataModel->rowCount() == 1) {
        QStringList tmpList;
        for (int i = 0; i < subject->dataModel->rowCount(); ++i) {
            tmpList += subject->dataModel->data(subject->dataModel->index(i), Qt::DisplayRole).toStringList();
        }
        QVERIFY(tmpList == list);
    }

    list.clear();

    subject->setCandidates(list, "test");
    QVERIFY(subject->candidatesList == list);
    QVERIFY(subject->dataModel->rowCount() == 0);
}

QTEST_APPLESS_MAIN(Ut_WordRibbonDialog);


