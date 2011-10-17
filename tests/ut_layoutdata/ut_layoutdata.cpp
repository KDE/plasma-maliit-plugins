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

#include "ut_layoutdata.h"
#include "layoutdata.h"
#include "mimkeymodel.h"
#include "utils.h"

#include <MApplication>
#include <MTheme>
#include <QStringList>

void Ut_LayoutData::initTestCase()
{
    static int argc = 2;
    static char *app_name[2] = { (char *) "ut_layoutdata",
                                 (char *) "-software" };

    disableQtPlugins();
    app = new MApplication(argc, app_name);
}

void Ut_LayoutData::cleanupTestCase()
{
    delete app;
    app = 0;
}

void Ut_LayoutData::init()
{
}

void Ut_LayoutData::cleanup()
{
}

void Ut_LayoutData::testConstructFromString_data()
{
    QTest::addColumn<QString>("characters");
    QTest::addColumn<int>("expectedColumnCount");
    QTest::addColumn<int>("expectedRowCount");
    QTest::addColumn<QStringList>("expectedLabels");

    const QStringList salmon(QStringList() << "s" << "a" << "l" << "m" << "o" << "n");

    QTest::newRow("salmon - single row")
        << "salmon" << 6 << 1
        << salmon;

    QTest::newRow("salm\\non - two rows")
        << "salm\non" << 4 << 2
        << salmon;

    QTest::newRow("sa\\nlm\\non - three rows")
        << "sa\nlm\non" << 2 << 3
        << salmon;

    QTest::newRow("salmon catfish trout - three keys")
        << "salmon catfish trout" << 3 << 1
        << (QStringList() << "salmon" << "catfish" << "trout");


    QTest::newRow("salmon catfish trout\\ncarp perch - five keys over two rows")
        << "salmon catfish trout\ncarp perch" << 3 << 2
        << (QStringList() << "salmon" << "catfish" << "trout" << "carp" << "perch");
}

void Ut_LayoutData::testConstructFromString()
{
    QFETCH(QString, characters);
    QFETCH(int, expectedColumnCount);
    QFETCH(int, expectedRowCount);
    QFETCH(QStringList, expectedLabels);

    const LayoutSection section(characters, false);

    QCOMPARE(section.maxColumns(), expectedColumnCount);
    QCOMPARE(section.rowCount(), expectedRowCount);
    QCOMPARE(section.keyCount(), expectedLabels.count());

    int labelIndex = 0;
    for (int rowIndex = 0; rowIndex < section.rowCount(); ++rowIndex) {
        for (int colIndex = 0; colIndex < section.maxColumns(); ++colIndex) {
            if (const MImKeyModel * const key = section.keyModel(rowIndex, colIndex)) {
                QCOMPARE(key->style(), MImKeyModel::NormalStyle);
                QCOMPARE(key->width(), MImKeyModel::Medium);
                QCOMPARE(key->rtl(), false);
                QCOMPARE(key->binding(true)->label(), expectedLabels.at(labelIndex));
                QCOMPARE(key->binding(false)->label(), expectedLabels.at(labelIndex));
                QCOMPARE(key->binding(false)->secondaryLabel(), QString());
                QVERIFY(!key->binding(false)->isDead());
                QCOMPARE(key->binding(false)->extendedLabels(), QString());
                QCOMPARE(key->binding(false)->action(), MImKeyBinding::ActionInsert);
                ++labelIndex;
            }
        }
    }

    // Verify that we actually found the expected keys:
    QCOMPARE(labelIndex, section.keyCount());
}

QTEST_APPLESS_MAIN(Ut_LayoutData);
