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

#include "ut_magnifierhost.h"
#include "utils.h"
#include "magnifierhost.h"
#include "mplainwindow.h"
#include "regiontracker.h"
#include "keyevent.h"
#include "mimabstractkeyarea.h"
#include "mimabstractkey.h"
#include "mimkey.h"
#include "reactionmappainter.h"

#include <QtCore>
#include <QtGui>
#include <MApplication>

Q_DECLARE_METATYPE(KeyEvent);
Q_DECLARE_METATYPE(MImAbstractKey*);
Q_DECLARE_METATYPE(const MImAbstractKey*);
Q_DECLARE_METATYPE(MImAbstractKey::ButtonState);

void Ut_MagnifierHost::initTestCase()
{
    static int argc = 2;
    static char *app_name[2] = { (char *) "ut_magnifierhost",
                                 (char *) "-software"};

    disableQtPlugins();
    app.reset(new MApplication(argc, app_name));

    RegionTracker::createInstance();
    ReactionMapPainter::createInstance();
    widget.reset(createMSceneWindow(new MPlainWindow));

    qRegisterMetaType<KeyEvent>();
    qRegisterMetaType<const MImAbstractKey*>();
    qRegisterMetaType<MImAbstractKey*>();
    qRegisterMetaType<MImAbstractKey::ButtonState>();
    qRegisterMetaType<KeyContext>("KeyContext");
}

void Ut_MagnifierHost::cleanupTestCase()
{}

void Ut_MagnifierHost::init()
{}

void Ut_MagnifierHost::cleanup()
{}

void Ut_MagnifierHost::testExtendedKeys_data()
{
    QTest::addColumn<QString>("keyAreaLabels");
    QTest::addColumn<QString>("extendedLabels");
    QTest::addColumn<int>("expectedRowCount");
    QTest::addColumn<int>("expectedKeyCount");

    // Using | and [, ] to draw ASCII keys in description, where [] means no key.
    QTest::newRow("'abc' + '' = []")
            << "abc" << "" << 0 << 0;

    QTest::newRow("'a' + 'bc' = [a|b|c]")
            << "a" << "bc" << 1 << 3;

    QTest::newRow("'abc' + 'def' = [abc|d|e|f]")
            << "abc" << "def" << 1 << 4 ;

    QTest::newRow("'abc' + 'd e f' = [abc|d|e|f]")
            << "abc" << "d e f" << 1 << 4;

    QTest::newRow("'some word' + 'def' = [some|word|d|e|f]")
            << "some word" << "def" << 1 << 5;

    QTest::newRow("'some word' + 'd\\nef' = [some|word|d]\\n[e|f]")
            << "some word" << "d\nef" << 2 << 5;

    QTest::newRow("'some word' + 'and more\\nwords here' = [some|word|and|more]\\n[words|here]")
            << "some word" << "and more\nwords here" << 2 << 6;
}

void Ut_MagnifierHost::testExtendedKeys()
{
    QFETCH(QString, keyAreaLabels);
    QFETCH(QString, extendedLabels);
    QFETCH(int, expectedRowCount);
    QFETCH(int, expectedKeyCount);
    const bool createsExtendedKeyArea(expectedKeyCount > 0);

    MagnifierHost host;
    host.setMainArea(MImKeyArea::create(LayoutData::SharedLayoutSection(new LayoutSection(keyAreaLabels)), true, 0));

    // Constructing a key. Yes, it's really that complex, for the most trivial case ...
    MImKeyBinding *binding = new MImKeyBinding; // model takes ownership and this is *not* a value type ...
    // Violating the private parts of a key binding, because there is just no other way to do it!
    binding->keyLabel = keyAreaLabels;
    binding->extended_labels = extendedLabels;
    binding->keyAction = MImKeyBinding::ActionInsert;

    MImKeyModel model;
    model.setBinding(*binding, false); // But when assigning the bindings, we pretend they were value types.
    model.setBinding(*binding, true);
    MImAbstractKeyAreaStyleContainer style;
    QSharedPointer<MImKey::StylingCache> cache(new MImKey::StylingCache);
    MImFontPool pool(true);
    MImKey key(model, style, *widget, cache, pool);
    QVERIFY(key.isNormalKey());
    KeyContext context(false);

    host.handleLongKeyPressedOnMainArea(&key, context);

    if (createsExtendedKeyArea) {
        MImAbstractKeyArea *keyArea = host.extendedKeysArea();
        QVERIFY(keyArea);

        QCOMPARE(keyArea->sectionModel()->rowCount(), expectedRowCount);
        QCOMPARE(keyArea->sectionModel()->keyCount(), expectedKeyCount);
    } else {
        QVERIFY(not host.extendedKeysArea());
    }
}

QTEST_APPLESS_MAIN(Ut_MagnifierHost);
