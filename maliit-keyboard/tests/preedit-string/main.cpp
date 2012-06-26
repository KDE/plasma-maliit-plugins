/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
 *
 */

#include "utils.h"

#include "models/area.h"
#include "models/key.h"
#include "models/keyarea.h"
#include "models/layout.h"
#include "models/text.h"
#include "view/glass.h"
#include "view/setup.h"
#include "plugin/editor.h"
#include "inputmethodhostprobe.h"
#include "wordengineprobe.h"

#include <maliit/plugins/testsurfacefactory.h>

#include <QtCore>
#include <QtTest>
#include <QWidget>
#include <QList>
#include <QMouseEvent>

using namespace MaliitKeyboard;
Q_DECLARE_METATYPE(Layout::Orientation)
Q_DECLARE_METATYPE(QList<QMouseEvent*>)

namespace {
const int g_size = 48;
const int g_divider = 3;

QPoint keyOriginLookup(const QString &name)
{
    static const int distance = g_size / g_divider;

    if (name == "a") {
        return QPoint(0, 0);
    } else if (name == "b") {
        return QPoint(distance, 0);
    } else if (name == "c") {
        return QPoint(0, distance);
    } else if (name == "d") {
        return QPoint(distance, distance);
    } else if (name == "space") {
        return QPoint(distance * 2, 0);
    } else if (name == "return") {
        return QPoint(distance * 2, distance);
    }

    return QPoint();
}

Key createKey(Key::Action action,
              const QString &text)
{
    static const QSize size(g_size / g_divider, g_size / g_divider);

    Key result;
    result.setAction(action);
    result.setOrigin(keyOriginLookup(text));
    result.rArea().setSize(size);
    result.rLabel().setText(text);

    return result;
}

// Populate KeyArea with six keys, a, b, c, d, space and return. Notice how the KeyArea
// covers the whole widget. Key width and height equals g_size / g_divider.
// .-----------.
// | a | b |<s>|
// |---|---|---|
// | c | d |<r>|
// `-----------'
KeyArea createAbcdArea()
{
    KeyArea key_area;
    Area area;
    area.setSize(QSize(g_size, g_size));
    key_area.setArea(area);

    key_area.rKeys().append(createKey(Key::ActionInsert, "a"));
    key_area.rKeys().append(createKey(Key::ActionInsert, "b"));
    key_area.rKeys().append(createKey(Key::ActionInsert, "c"));
    key_area.rKeys().append(createKey(Key::ActionInsert, "d"));
    key_area.rKeys().append(createKey(Key::ActionSpace,  "space"));
    key_area.rKeys().append(createKey(Key::ActionReturn, "return"));

    return key_area;
}

QList<QMouseEvent *> createPressReleaseEvent(const QPoint &origin,
                                             Layout::Orientation orientation)
{
    static const int offset(g_size / (g_divider * 2));

    QList<QMouseEvent *> result;
    QPoint pos = (orientation == Layout::Landscape) ? QPoint(origin.x() + offset, origin.y() + offset)
                                                    : QPoint(origin.y() + offset, g_size - (origin.x() + offset));

    result.append(new QMouseEvent(QKeyEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
    result.append(new QMouseEvent(QKeyEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));

    return result;
}

bool operator==(const Maliit::PreeditTextFormat &a, const Maliit::PreeditTextFormat &b) {
    return ((a.start == b.start) and (a.length == b.length) and (a.preeditFace == b.preeditFace));
}

} // unnamed namespace


class TestPreeditString
    : public QObject
{
    Q_OBJECT

private:
    typedef QList<Maliit::PreeditTextFormat> FormatList;

    Q_SLOT void initTestCase()
    {
        qRegisterMetaType<QList<QMouseEvent*> >();
        qRegisterMetaType<Layout::Orientation>();
        qRegisterMetaType<FormatList>();
    }

    Q_SLOT void test_data()
    {
        QTest::addColumn<Layout::Orientation>("orientation");
        QTest::addColumn<QList<QMouseEvent*> >("mouse_events");
        QTest::addColumn<QString>("expected_last_preedit_string");
        QTest::addColumn<QString>("expected_commit_string");
        QTest::addColumn<FormatList>("expected_preedit_format");
        QTest::addColumn<bool>("word_engine_enabled");

        for (int orientation = 0; orientation < 1; ++orientation) {
            // FIXME: here should be 2          ^
            // FIXME: tests fail for portrait layouts
            const Layout::Orientation layout_orientation(orientation == 0 ? Layout::Landscape
                                                                          : Layout::Portrait);
            QTest::newRow("No mouse events: expect empty commit string, should be no preedit face")
                << layout_orientation
                << (QList<QMouseEvent *>())
                << "" << "" << FormatList() << true;

            QTest::newRow("Only return pressed: expect empty commit string, should be no preedit face")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("return"), layout_orientation))
                << "" << "" << FormatList() << true;

            QTest::newRow("Release outside of widget: expect empty commit string, should be no preedit face")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(QPoint(g_size * 2, g_size * 2), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("return"), layout_orientation))
                << "" << "" << FormatList() << true;

            QTest::newRow("Release button over key 'a': expect commit string 'a', preedit face should be active.")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("a"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("return"), layout_orientation))
                << "a" << "a" << (FormatList() << Maliit::PreeditTextFormat(0, 1, Maliit::PreeditActive)) << true;

            QTest::newRow("Release button over key 'a', but no commit: expect empty commit string.")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("a"), layout_orientation))
                << "a" << "" << (FormatList() << Maliit::PreeditTextFormat(0, 1, Maliit::PreeditActive)) << true;

            QTest::newRow("Release button over keys 'c, b, d, a': expect commit string 'cbda', preedit face should be no candidates")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("c"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("b"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("d"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("a"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("space"), layout_orientation))
                << "cbda" << "cbda " << (FormatList() << Maliit::PreeditTextFormat(0, 4, Maliit::PreeditNoCandidates)) << true;

            QTest::newRow("Typing two words: expect commit string 'ab cd', with last preedit being 'cd', preedit face should be no candidates.")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("a"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("b"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("space"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("c"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("d"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("return"), layout_orientation))
                << "cd" << "ab cd" << (FormatList() << Maliit::PreeditTextFormat(0, 2, Maliit::PreeditNoCandidates)) << true;

            QTest::newRow("Typing one word 'abd': expect commit string 'abd', with last preedit being 'abd', preedit face should be default")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("a"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("b"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("d"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("return"), layout_orientation))
                << "abd" << "abd" << (FormatList() << Maliit::PreeditTextFormat(0, 3, Maliit::PreeditDefault)) << true;

            // TODO: we probably should not sent any preedit formats when word engine is turned off.
            QTest::newRow("Typing one word 'abd' with word engine turned off: expect commit string 'abd', with preedit being last char, should be no preedit face")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("a"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("b"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("d"), layout_orientation))
                << "d" << "abd" << (FormatList() << Maliit::PreeditTextFormat(0, 1, Maliit::PreeditDefault)) << false;

            QTest::newRow("Typing one word 'ab' with word engine turned off: expect commit string 'ab', with preedit being last char, should be no preedit face")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("a"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("b"), layout_orientation))
                << "b" << "ab" << (FormatList() << Maliit::PreeditTextFormat(0, 1, Maliit::PreeditDefault)) << false;

            QTest::newRow("Typing one word 'bd' with word engine turned off: expect commit string 'bd', with preedit being last char, face should be for one char with default face")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("b"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("d"), layout_orientation))
                << "d" << "bd" << (FormatList() << Maliit::PreeditTextFormat(0, 1, Maliit::PreeditDefault)) << false;

        }
    }

    Q_SLOT void test()
    {
        // FIXME: mikhas: We should have tests for the preedit &
        // preedit correctness stuff, and how it blends with word
        // prediction. I guess you will need to add
        // WordEngine::setSpellChecker() API so that you can inject a
        // fake spellchecker, for the tests. Otherwise, the test would
        // have to be skipped when there's no hunspell/presage, which
        // I wouldn't like to have.

        QFETCH(Layout::Orientation, orientation);
        QFETCH(QList<QMouseEvent*>, mouse_events);
        QFETCH(QString, expected_last_preedit_string);
        QFETCH(QString, expected_commit_string);
        QFETCH(QList<Maliit::PreeditTextFormat>, expected_preedit_format);
        QFETCH(bool, word_engine_enabled);

        Glass glass;
        Editor editor(EditorOptions(), new Model::Text, new Logic::WordEngineProbe, 0);
        InputMethodHostProbe host;
        SharedLayout layout(new Layout);
        QSharedPointer<Maliit::Plugins::AbstractGraphicsViewSurface> surface(Maliit::Plugins::createTestGraphicsViewSurface());
        QSharedPointer<Maliit::Plugins::AbstractGraphicsViewSurface> extended_surface(Maliit::Plugins::createTestGraphicsViewSurface(surface));

        // geometry stuff is usually done by maliit-server, so we need
        // to do it manually here:
        //surface->view()->viewport()->setGeometry(0, 0, g_size, g_size);
        surface->view()->setSceneRect(0, 0, g_size, g_size);
        surface->scene()->setSceneRect(0, 0, g_size, g_size);
        glass.setSurface(surface);
        glass.setExtendedSurface(extended_surface);
        glass.addLayout(layout);
        editor.setHost(&host);
        layout->setOrientation(orientation);
        editor.wordEngine()->setEnabled(word_engine_enabled);

        Setup::connectGlassToTextEditor(&glass, &editor);
        const KeyArea &key_area(createAbcdArea());

        layout->setExtendedPanel(key_area);
        layout->setActivePanel(Layout::ExtendedPanel);

        Q_FOREACH (QMouseEvent *ev, mouse_events) {
            QApplication::instance()->postEvent(surface->view()->viewport(), ev);
        }

        TestUtils::waitForSignal(&glass, SIGNAL(keyReleased(Key,SharedLayout)));
        QCOMPARE(host.lastPreeditString(), expected_last_preedit_string);
        QCOMPARE(host.commitStringHistory(), expected_commit_string);
        QCOMPARE(host.lastPreeditTextFormatList(), expected_preedit_format);
    }
};

QTEST_MAIN(TestPreeditString)
#include "main.moc"
