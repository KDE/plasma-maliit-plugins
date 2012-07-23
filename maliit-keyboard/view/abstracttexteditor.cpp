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

namespace {

//! Checks whether given \a c is a word separator.
//! \param c char to test
//!
//! Other way to do checks would be using isLetterOrNumber() + some
//! other methods. But UTF is so crazy that I am not sure whether
//! other strange categories are parts of the word or not. It is
//! easier to specify punctuations and whitespaces.
inline bool isSeparator(const QChar &c)
{
    return (c.isPunct() or c.isSpace());
}

//! Extracts a word boundaries at cursor position.
//! \param surrounding_text text from which extraction will happen
//! \param cursor_position position of cursor within \a surrounding_text
//! \param replacement place where replacement data will be stored
//!
//! \return whether surrounding text was valid (not empty).
//!
//! If cursor is placed right after the word, boundaries of this word
//! are extracted.  Otherwise if cursor is placed right before the
//! word, then no word boundaries are stored - instead invalid
//! replacement is stored. It might happen that cursor position is
//! outside the string, so \a replacement will have fixed position.
bool extractWordBoundariesAtCursor(const QString& surrounding_text,
                                   int cursor_position,
                                   AbstractTextEditor::Replacement *replacement)
{
    const int text_length(surrounding_text.length());

    if (text_length == 0) {
        return false;
    }

    // just in case - if cursor is far after last char in surrounding
    // text we place it right after last char.
    cursor_position = qBound(0, cursor_position, text_length);

    // cursor might be placed in after last char (that is to say - its
    // index might be the one of string terminator) - for simplifying
    // the algorithm below we fake it as cursor is put on delimiter:
    // "abc" - surrounding text
    //     | - cursor placement
    // "abc " - fake surrounding text
    const QString fake_surrounding_text(surrounding_text + " ");
    const QChar *const fake_data(fake_surrounding_text.constData());
    // begin is index of first char in a word
    int begin(-1);
    // end is index of a char after last char in a word.
    // -2, because -2 - (-1) = -1 and we would like to
    // have -1 as invalid length.
    int end(-2);

    for (int iter(cursor_position); iter >= 0; --iter) {
        const QChar &c(fake_data[iter]);

        if (isSeparator(c)) {
            if (iter != cursor_position) {
                break;
            }
        } else {
            begin = iter;
        }
    }

    if (begin >= 0) {
        // take note that fake_data's last QChar is always a space.
        for (int iter(cursor_position); iter <= text_length; ++iter) {
            const QChar &c(fake_data[iter]);

            end = iter;
            if (isSeparator(c)) {
                break;
            }
        }
    }

    if (replacement) {
        replacement->start = begin;
        replacement->length = end - begin;
        replacement->cursor_position = cursor_position;
    }

    return true;
}

} // unnamed namespace

EditorOptions::EditorOptions()
    : backspace_auto_repeat_delay(500)
    , backspace_auto_repeat_interval(300)
{}


QString autoAppendix(const QString &preedit,
                     bool *auto_caps_activated)
{
    if (not auto_caps_activated) {
        return QString(" ");
    }

    *auto_caps_activated = false;
    const QString &last_char(preedit.right(1));
    QString appendix;

    if (not last_char.isEmpty()
        && last_char.at(0).isPunct()) {
        *auto_caps_activated = true;
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
    int ignore_next_cursor_position;
    QString ignore_next_surrounding_text;

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
    , ignore_next_cursor_position(-1)
    , ignore_next_surrounding_text()
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

    connect(word_engine, SIGNAL(candidatesChanged(WordCandidateList)),
            this,        SIGNAL(wordCandidatesChanged(WordCandidateList)));

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
        if (d->auto_correct_enabled && not d->text->primaryCandidate().isEmpty()) {
            d->text->setPrimaryCandidate(QString());
            d->backspace_sent = true;
        } else {
            d->backspace_sent = false;
        }

        commitPreedit();
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

        sendPreeditString(d->text->preedit(), d->text->preeditFace(),
                          Replacement(d->text->cursorPosition()));

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
        bool auto_caps_activated = false;
        const QString &appendix(autoAppendix(d->text->preedit(), &auto_caps_activated));

        if (d->auto_correct_enabled && not d->text->primaryCandidate().isEmpty()) {
            d->text->setPreedit(d->text->primaryCandidate());
        }

        d->text->appendToPreedit(appendix);
        commitPreedit();

        if (auto_caps_activated) {
            Q_EMIT autoCapsActivated();
        }
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

    bool auto_caps_activated = false;
    const QString &appendix(autoAppendix(d->text->preedit(), &auto_caps_activated));
    d->text->setPreedit(replacement);
    d->text->appendToPreedit(appendix);
    commitPreedit();

    if (auto_caps_activated) {
        Q_EMIT autoCapsActivated();
    }
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

void AbstractTextEditor::showUserCandidate()
{
    Q_D(AbstractTextEditor);

    WordCandidateList candidates;
    WordCandidate candidate(WordCandidate::SourceUser, d->text->preedit());

    candidates << candidate;

    Q_EMIT wordCandidatesChanged(candidates);
}

void AbstractTextEditor::addToUserDictionary(const QString &word)
{
    Q_D(AbstractTextEditor);

    d->word_engine->addToUserDictionary(word);
    d->text->setPrimaryCandidate(word);

    Q_EMIT wordCandidatesChanged(WordCandidateList());
}

void AbstractTextEditor::sendPreeditString(const QString &preedit,
                                           Model::Text::PreeditFace face)
{
    sendPreeditString(preedit, face, Replacement());
}

void AbstractTextEditor::onCursorPositionChanged(int cursor_position,
                                                 const QString &surrounding_text)
{
    Q_D(AbstractTextEditor);
    Replacement r;

    if (not extractWordBoundariesAtCursor(surrounding_text, cursor_position, &r)) {
        return;
    }

    if (r.start < 0 or r.length < 0) {
        if (d->ignore_next_surrounding_text == surrounding_text and
            d->ignore_next_cursor_position == cursor_position) {
            d->ignore_next_surrounding_text.clear();
            d->ignore_next_cursor_position = -1;
        } else {
            d->text->setPreedit("");
            d->text->setCursorPosition(0);
        }
    } else {
        const int cursor_pos_relative_word_begin(r.start - r.cursor_position);
        const int word_begin_relative_cursor_pos(r.cursor_position - r.start);
        const QString word(surrounding_text.mid(r.start, r.length));
        Replacement word_r(cursor_pos_relative_word_begin, r.length,
                           word_begin_relative_cursor_pos);

        d->text->setPreedit(word, word_begin_relative_cursor_pos);
        // computeCandidates can change preedit face, so needs to happen
        // before sending preedit:
        d->word_engine->computeCandidates(d->text.data());
        sendPreeditString(d->text->preedit(), d->text->preeditFace(), word_r);
        // Qt is going to send us an event with cursor position places
        // at the beginning of replaced word and surrounding text
        // without the replaced word. We want to ignore it.
        d->ignore_next_cursor_position = r.start;
        d->ignore_next_surrounding_text = QString(surrounding_text).remove(r.start, r.length);
    }
}

} // namespace MaliitKeyboard
