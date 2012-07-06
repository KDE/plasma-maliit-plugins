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

#include "setup.h"
#include "glass.h"
#include "renderer.h"
#include "abstracttexteditor.h"
#include "abstractfeedback.h"

#include "models/key.h"
#include "models/wordcandidate.h"
#include "models/text.h"

#include "logic/layout.h"
#include "logic/layoutupdater.h"

namespace MaliitKeyboard {
namespace Setup {

void connectAll(Glass *glass,
                Logic::Layout *layout,
                Logic::LayoutUpdater *updater,
                Renderer *renderer,
                AbstractTextEditor *editor,
                AbstractFeedback *feedback)
{
    connectGlassToLayoutUpdater(glass, updater);
    connectGlassToTextEditor(glass, editor);
    connectGlassToFeedback(glass, feedback);

    connectLayoutToRenderer(layout, renderer);
    connectLayoutUpdaterToTextEditor(updater, editor);
}

void connectGlassToLayoutUpdater(Glass *glass,
                                 Logic::LayoutUpdater *updater)
{
    QObject::connect(glass,   SIGNAL(switchLeft(Logic::Layout *)),
                     updater, SLOT(clearActiveKeysAndMagnifier()));

    QObject::connect(glass,   SIGNAL(switchRight(Logic::Layout *)),
                     updater, SLOT(clearActiveKeysAndMagnifier()));

    // Connect key signals to key signal handlers:
    QObject::connect(glass,   SIGNAL(keyPressed(Key,Logic::Layout *)),
                     updater, SLOT(onKeyPressed(Key,Logic::Layout *)));

    QObject::connect(glass,   SIGNAL(keyLongPressed(Key,Logic::Layout *)),
                     updater, SLOT(onKeyLongPressed(Key,Logic::Layout *)));

    QObject::connect(glass,   SIGNAL(keyReleased(Key,Logic::Layout *)),
                     updater, SLOT(onKeyReleased(Key,Logic::Layout *)));

    QObject::connect(glass,   SIGNAL(keyAreaPressed(Logic::Layout::Panel,Logic::Layout *)),
                     updater, SLOT(onKeyAreaPressed(Logic::Layout::Panel,Logic::Layout *)));

    QObject::connect(glass,   SIGNAL(keyAreaReleased(Logic::Layout::Panel,Logic::Layout *)),
                     updater, SLOT(onKeyAreaReleased(Logic::Layout::Panel,Logic::Layout *)));

    QObject::connect(glass,   SIGNAL(keyEntered(Key,Logic::Layout *)),
                     updater, SLOT(onKeyEntered(Key,Logic::Layout *)));

    QObject::connect(glass,   SIGNAL(keyExited(Key,Logic::Layout *)),
                     updater, SLOT(onKeyExited(Key,Logic::Layout *)));

    // Connect word candidate signals to word candidate handlers:
    QObject::connect(glass,   SIGNAL(wordCandidatePressed(WordCandidate,Logic::Layout *)),
                     updater, SLOT(onWordCandidatePressed(WordCandidate,Logic::Layout *)));

    QObject::connect(glass,   SIGNAL(wordCandidateReleased(WordCandidate,Logic::Layout *)),
                     updater, SLOT(onWordCandidateReleased(WordCandidate,Logic::Layout *)));
}

void connectGlassToTextEditor(Glass *glass,
                              AbstractTextEditor *editor)
{
    QObject::connect(glass,  SIGNAL(keyPressed(Key,Logic::Layout *)),
                     editor, SLOT(onKeyPressed(Key)));

    QObject::connect(glass,  SIGNAL(keyReleased(Key,Logic::Layout *)),
                     editor, SLOT(onKeyReleased(Key)));

    QObject::connect(glass,  SIGNAL(keyEntered(Key,Logic::Layout *)),
                     editor, SLOT(onKeyEntered(Key)));

    QObject::connect(glass,  SIGNAL(keyExited(Key,Logic::Layout *)),
                     editor, SLOT(onKeyExited(Key)));

    QObject::connect(editor, SIGNAL(keyboardClosed()),
                     glass,  SIGNAL(keyboardClosed()));
}

void connectGlassToFeedback (Glass *glass,
                             AbstractFeedback *feedback)
{
    QObject::connect(glass,    SIGNAL(keyPressed(Key,Logic::Layout *)),
                     feedback, SLOT(onKeyPressed()));
    QObject::connect(glass,    SIGNAL(keyReleased(Key,Logic::Layout *)),
                     feedback, SLOT(onKeyReleased()));
    QObject::connect(glass,    SIGNAL(switchLeft(Logic::Layout *)),
                     feedback, SLOT(onLayoutChanged()));
    QObject::connect(glass,    SIGNAL(switchRight(Logic::Layout *)),
                     feedback, SLOT(onLayoutChanged()));
    QObject::connect(glass,    SIGNAL(keyboardClosed()),
                     feedback, SLOT(onKeyboardHidden()));
}

void connectLayoutToRenderer(Logic::Layout *layout,
                             Renderer *renderer)
{
    QObject::connect(layout,   SIGNAL(magnifierKeyChanged(Key)),
                     renderer, SLOT(onMagnifierKeyChanged(Key)));

    QObject::connect(layout,   SIGNAL(activeKeysChanged(QVector<Key>)),
                     renderer, SLOT(onActiveKeysChanged(QVector<Key>)));

    QObject::connect(layout,   SIGNAL(activeExtendedKeysChanged(QVector<Key>)),
                     renderer, SLOT(onActiveExtendedKeysChanged(QVector<Key>)));

    QObject::connect(layout,   SIGNAL(centerPanelChanged(KeyArea,QPoint)),
                     renderer, SLOT(onCenterPanelChanged(KeyArea,QPoint)));

    QObject::connect(layout,   SIGNAL(extendedPanelChanged(KeyArea,QPoint)),
                     renderer, SLOT(onExtendedPanelChanged(KeyArea,QPoint)));

    QObject::connect(layout,   SIGNAL(wordRibbonChanged(WordRibbon,QRect)),
                     renderer, SLOT(onWordRibbonChanged(WordRibbon,QRect)));
}

void connectLayoutUpdaterToTextEditor(Logic::LayoutUpdater *updater,
                                      AbstractTextEditor *editor)
{
    QObject::connect(updater, SIGNAL(wordCandidateSelected(QString)),
                     editor,  SLOT(replacePreedit(QString)));

    QObject::connect(editor,  SIGNAL(preeditEnabledChanged(bool)),
                     updater, SLOT(setWordRibbonVisible(bool)));

    QObject::connect(editor,  SIGNAL(wordCandidatesChanged(QStringList)),
                     updater, SLOT(onWordCandidatesChanged(QStringList)));
}

}} // namespace Setup, MaliitKeyboard
