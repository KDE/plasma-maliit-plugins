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

void Ut_LayoutData::testConstructFromString()
{
    const QString characters("salmon");
    const LayoutSection section(characters, false);

    QCOMPARE(section.maxColumns(), characters.length());
    QCOMPARE(section.rowCount(), 1);
    QCOMPARE(section.keyCount(), characters.length());
    QCOMPARE(section.columnsAt(0), characters.length());
    for (int i = 0; i < characters.length(); ++i) {
        const MImKeyModel * const key(section.keyModel(0, i));
        QVERIFY(key);
        QCOMPARE(key->style(), MImKeyModel::NormalStyle);
        QCOMPARE(key->width(), MImKeyModel::Medium);
        QVERIFY(key->isFixedWidth());
        QCOMPARE(key->rtl(), false);
        QCOMPARE(key->binding(false)->label(), QString(characters[i]));
        QCOMPARE(key->binding(true)->label(), QString(characters[i]));
        QCOMPARE(key->binding(false)->secondaryLabel(), QString());
        QVERIFY(!key->binding(false)->isDead());
        QCOMPARE(key->binding(false)->extendedLabels(), QString());
        QCOMPARE(key->binding(false)->action(), MImKeyBinding::ActionInsert);
    }
}


QTEST_APPLESS_MAIN(Ut_LayoutData);
