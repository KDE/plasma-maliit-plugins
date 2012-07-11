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

#include "abstracttexteditor.h"
#include "models/wordribbon.h"

namespace MaliitKeyboard {

EditorOptions::EditorOptions()
    : backspace_auto_repeat_delay(500)
    , backspace_auto_repeat_interval(300)
{}


QString autoAppendix(const QString &preedit)
{
    const QString &last_char(preedit.right(1));
    QString appendix;

    if (not last_char.isEmpty()
        && not last_char.at(0).isLetterOrNumber()) {
        appendix.append(last_char);
    }

    appendix.append(" ");
    return appendix;
}


class AbstractTextEditorPrivate
{
public:
    QTimer auto_repeat_backspace_timer;
    bool backspace_sent;
    EditorOptions options;
    QScopedPointer<Model::Text> text;
    QScopedPointer<Logic::AbstractWordEngine> word_engine;
    bool preedit_enabled;
    bool auto_correct_enabled;

    explicit AbstractTextEditorPrivate(const EditorOptions &new_options,
                                       Model::Text *new_text,
                                       Logic::AbstractWordEngine *new_word_engine);
    bool valid() const;
};

AbstractTextEditorPrivate::AbstractTextEditorPrivate(const EditorOptions &new_options,
                                                     Model::Text *new_text,
                                                     Logic::AbstractWordEngine *new_word_engine)
    : auto_repeat_backspace_timer()
    , backspace_sent(false)
    , options(new_options)
    , text(new_text)
    , word_engine(new_word_engine)
    , preedit_enabled(false)
    , auto_correct_enabled(false)
{
    auto_repeat_backspace_timer.setSingleShot(true);
    (void) valid();
}

bool AbstractTextEditorPrivate::valid() const
{
    const bool is_invalid(text.isNull() || word_engine.isNull());

    if (is_invalid) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Invalid text model, or no word engine given! The text editor will not function properly.";
    }

    return (not is_invalid);
}

AbstractTextEditor::AbstractTextEditor(const EditorOptions &options,
                                       Model::Text *text,
                                       Logic::AbstractWordEngine *word_engine,
                                       QObject *parent)
    : QObject(parent)
    , d_ptr(new AbstractTextEditorPrivate(options, text, word_engine))
{
    connect(&d_ptr->auto_repeat_backspace_timer, SIGNAL(timeout()),
            this,                                SLOT(autoRepeatBackspace()));

    connect(word_engine, SIGNAL(enabledChanged(bool)),
            this,        SLOT(setPreeditEnabled(bool)));

    connect(word_engine, SIGNAL(candidatesChanged(QStringList)),
            this,        SIGNAL(wordCandidatesChanged(QStringList)));

    setPreeditEnabled(word_engine->isEnabled());
}

AbstractTextEditor::~AbstractTextEditor()
{}

Model::Text * AbstractTextEditor::text() const
{
    Q_D(const AbstractTextEditor);
    return d->text.data();
}

Logic::AbstractWordEngine * AbstractTextEditor::wordEngine() const
{
    Q_D(const AbstractTextEditor);
    return d->word_engine.data();
}

void AbstractTextEditor::onKeyPressed(const Key &key)
{
    Q_D(AbstractTextEditor);

    if (not d->valid()) {
        return;
    }

    if (key.action() == Key::ActionBackspace) {
        sendCommitString(d->text->preedit());
        d->text->commitPreedit();
        d->backspace_sent = false;
        d->auto_repeat_backspace_timer.start(d->options.backspace_auto_repeat_delay);
    }
}

void AbstractTextEditor::onKeyReleased(const Key &key)
{
    Q_D(AbstractTextEditor);

    if (not d->valid()) {
        return;
    }

    const QString &text(key.label().text());
    Qt::Key event_key = Qt::Key_unknown;

    switch(key.action()) {
    case Key::ActionInsert:
        d->text->appendToPreedit(text);
#ifdef DISABLE_PREEDIT
        commitPreedit();
#else
        sendPreeditString(d->text->preedit(), d->text->preeditFace());
#endif

        // computeCandidates can change preedit face, so needs to happen
        // before sending preedit:
        if (d->preedit_enabled) {
            d->word_engine->computeCandidates(d->text.data());
        }

        sendPreeditString(d->text->preedit(), d->text->preeditFace());

        if (not d->preedit_enabled) {
            commitPreedit();
        }

        break;

    case Key::ActionBackspace: {
        commitPreedit();

        if (not d->backspace_sent) {
            event_key = Qt::Key_Backspace;
        }

        d->auto_repeat_backspace_timer.stop();
     } break;

    case Key::ActionSpace: {
        const QString &appendix(autoAppendix(d->text->preedit()));

        if (d->auto_correct_enabled && not d->text->primaryCandidate().isEmpty()) {
            d->text->setPreedit(d->text->primaryCandidate());
        }

        d->text->appendToPreedit(appendix);
        commitPreedit();  
    } break;

    case Key::ActionReturn:
        event_key = Qt::Key_Return;
        break;

    case Key::ActionClose:
        Q_EMIT keyboardClosed();
        break;

    case Key::ActionLeft:
        event_key = Qt::Key_Left;
        break;

    case Key::ActionUp:
        event_key = Qt::Key_Up;
        break;

    case Key::ActionRight:
        event_key = Qt::Key_Right;
        break;

    case Key::ActionDown:
        event_key = Qt::Key_Down;
        break;

    case Key::ActionLeftLayout:
        Q_EMIT leftLayoutSelected();
        break;

    case Key::ActionRightLayout:
        Q_EMIT rightLayoutSelected();
        break;

    default:
        break;
    }

    if (event_key != Qt::Key_unknown) {
        commitPreedit();
        QKeyEvent ev(QEvent::KeyPress, event_key, Qt::NoModifier);
        sendKeyEvent(ev);
    }
}

void AbstractTextEditor::onKeyEntered(const Key &key)
{
    Q_D(AbstractTextEditor);

    if (key.action() == Key::ActionBackspace) {
        d->backspace_sent = false;
        d->auto_repeat_backspace_timer.start(d->options.backspace_auto_repeat_delay);
    }
}

void AbstractTextEditor::onKeyExited(const Key &key)
{
    Q_D(AbstractTextEditor);

    if (key.action() == Key::ActionBackspace) {
        d->auto_repeat_backspace_timer.stop();
    }
}

void AbstractTextEditor::replacePreedit(const QString &replacement)
{
    Q_D(AbstractTextEditor);

    if (not d->valid()) {
        return;
    }

    d->text->setPreedit(replacement);
    // computeCandidates can change preedit face, so needs to happen
    // before sending preedit:
    d->word_engine->computeCandidates(d->text.data());
    sendPreeditString(d->text->preedit(), d->text->preeditFace());
}

void AbstractTextEditor::replaceAndCommitPreedit(const QString &replacement)
{
    Q_D(AbstractTextEditor);

    if (not d->valid()) {
        return;
    }

    const QString &appendix(autoAppendix(d->text->preedit()));
    d->text->setPreedit(replacement);
    d->text->appendToPreedit(appendix);
    commitPreedit();
}

void AbstractTextEditor::clearPreedit()
{
    replacePreedit("");
}

bool AbstractTextEditor::isPreeditEnabled() const
{
    Q_D(const AbstractTextEditor);
    return d->preedit_enabled;
}

void AbstractTextEditor::setPreeditEnabled(bool enabled)
{
    Q_D(AbstractTextEditor);

    if (d->preedit_enabled != enabled) {
        d->preedit_enabled = enabled;
        Q_EMIT preeditEnabledChanged(d->preedit_enabled);
    }
}

bool AbstractTextEditor::isAutoCorrectEnabled() const
{
    Q_D(const AbstractTextEditor);
    return d->auto_correct_enabled;
}

void AbstractTextEditor::setAutoCorrectEnabled(bool enabled)
{
    Q_D(AbstractTextEditor);

    if (d->auto_correct_enabled != enabled) {
        d->auto_correct_enabled = enabled;
        Q_EMIT autoCorrectEnabledChanged(d->auto_correct_enabled);
    }
}

void AbstractTextEditor::commitPreedit()
{
    Q_D(AbstractTextEditor);

    if (not d->valid() || d->text->preedit().isEmpty()) {
        return;
    }

    sendCommitString(d->text->preedit());
    d->text->commitPreedit();
    d->word_engine->clearCandidates();
}

// TODO: this implementation does not take into account following features:
// 1) preedit string
//      if there is preedit then first call to autoRepeatBackspace should clean it completely
//      and following calls should remove remaining text character by character
// 2) multitouch
//      it is not completely clean how to handle multitouch for backspace,
//      but we can follow the strategy from meego-keyboard - release pressed
//      key when user press another one at the same time. Then we do not need to
//      change anything in this method
void AbstractTextEditor::autoRepeatBackspace()
{
    Q_D(AbstractTextEditor);

    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    sendKeyEvent(ev);
    d->backspace_sent = true;
    d->auto_repeat_backspace_timer.start(d->options.backspace_auto_repeat_interval);
}

} // namespace MaliitKeyboard
