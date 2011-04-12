/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list 
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list 
 * of conditions and the following disclaimer in the documentation and/or other materials 
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be 
 * used to endorse or promote products derived from this software without specific 
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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


