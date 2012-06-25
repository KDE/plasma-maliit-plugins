/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2012 Openismus GmbH. All rights reserved.
 *
 * Contact: maliit-discuss@lists.maliit.org
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
 *
 */

#include "utils.h"
#include "wordengineprobe.h"
#include "common/inputmethodhostprobe.h"

#include "plugin/editor.h"
#include "models/key.h"
#include "models/text.h"

#include <QtCore>
#include <QtTest>

using namespace MaliitKeyboard;

class TestWordCandidates
    : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void initTestCase()
    {}

    Q_SLOT void testPrediction_data()
    {
        QTest::addColumn<bool>("enable_preedit");
        QTest::addColumn<bool>("enable_auto_correct");
        QTest::addColumn<QString>("preedit");
        QTest::addColumn<QString>("expected_word_candidate");
        QTest::addColumn<QString>("expected_commit_history");

        QTest::newRow("preedit enabled")
                << true << false << "preedit" << "tideerp" << "preedit ";

        QTest::newRow("preedit + auto-correct enabled")
                << true << true << "preedit" << "tideerp" << "tideerp ";
    }

    Q_SLOT void testPrediction()
    {
        QFETCH(bool, enable_preedit);
        QFETCH(bool, enable_auto_correct);
        QFETCH(QString, preedit);
        QFETCH(QString, expected_word_candidate);
        QFETCH(QString, expected_commit_history);

        Editor editor(EditorOptions(), new Model::Text, new Logic::WordEngineProbe);
        QSignalSpy spy(&editor, SIGNAL(wordCandidatesChanged(QStringList)));
        QSignalSpy preedit_enabled_spy(&editor, SIGNAL(preeditEnabledChanged(bool)));
        QSignalSpy auto_correct_enabled_spy(&editor, SIGNAL(autoCorrectEnabledChanged(bool)));

        InputMethodHostProbe host;
        editor.setHost(&host);

        QVERIFY(not editor.isPreeditEnabled());
        editor.wordEngine()->setEnabled(enable_preedit);
        QCOMPARE(preedit_enabled_spy.count(), enable_preedit ? 1 : 0);
        QCOMPARE(editor.isPreeditEnabled(), enable_preedit);

        QVERIFY(not editor.isAutoCorrectEnabled());
        editor.setAutoCorrectEnabled(enable_auto_correct);
        QCOMPARE(auto_correct_enabled_spy.count(), enable_auto_correct ? 1 : 0);
        QCOMPARE(editor.isAutoCorrectEnabled(), enable_auto_correct);

        Q_FOREACH(const QChar &c, preedit) {
            Key k;
            QCOMPARE(k.action(), Key::ActionInsert);

            k.rLabel().setText(QString(c));
            editor.onKeyReleased(k);
        }

        QVERIFY(spy.count() > 0);
        QCOMPARE(spy.takeLast().first().toStringList().first(), expected_word_candidate);

        // Force a commit:
        Key space;
        space.setAction(Key::ActionSpace);
        editor.onKeyReleased(space);

        QCOMPARE(editor.text()->preedit(), QString());
        QCOMPARE(host.commitStringHistory(), expected_commit_history);

        QVERIFY(spy.count() > 0);
        QCOMPARE(spy.takeLast().first().toStringList(), QStringList());
    }
};

QTEST_MAIN(TestWordCandidates)
#include "main.moc"
