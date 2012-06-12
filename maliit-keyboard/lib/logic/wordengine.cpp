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

#include "wordengine.h"
#include "spellchecker.h"

#ifdef HAVE_PRESAGE
#include <presage.h>
#endif

//! \class MaliitKeyboard::Logic::WordEngine
//! Provides error correction based on preedit (default engine: hunspell) and
//! word prediction based on surrounding text and preedit (default engine:
//! presage). The word engine requires to be notifed on text model changes,
//! \sa MaliitKeyboard::Model::Text and
//! \sa MaliitKeyboard::Logic::WordEngine::onTextChanged().

//! \fn MaliitKeyboard::Logic::WordEngine::enabledChanged
//! \brief Emitted when word engine toggles word candidate updates on/off.
//! \param enabled Whether word engine is enabled.

//! \fn MaliitKeyboard::Logic::WordEngine::candidatesUpdated
//! \brief Emitted when new candidates have been computed.
//! \param candidates The list of updated candidates.
namespace MaliitKeyboard {
namespace Logic {

#ifdef HAVE_PRESAGE
class CandidatesCallback
    : public PresageCallback
{
private:
    const std::string& m_past_context;
    const std::string m_empty;

public:
    explicit CandidatesCallback(const std::string& past_context);

    std::string get_past_stream() const;
    std::string get_future_stream() const;
};

CandidatesCallback::CandidatesCallback(const std::string &past_context)
    : m_past_context(past_context)
    , m_empty()
{}

std::string CandidatesCallback::get_past_stream() const
{
    return m_past_context;
}

std::string CandidatesCallback::get_future_stream() const
{
    return m_empty;
}
#endif


class WordEnginePrivate
{
public:
    QStringList candidates;
    SpellChecker spell_checker;
    bool enabled;
#ifdef HAVE_PRESAGE
    std::string candidates_context;
    CandidatesCallback presage_candidates;
    Presage presage;
#endif

    explicit WordEnginePrivate();
};

WordEnginePrivate::WordEnginePrivate()
    : candidates()
    , spell_checker()
    , enabled(false)
#ifdef HAVE_PRESAGE
    , candidates_context()
    , presage_candidates(CandidatesCallback(candidates_context))
    , presage(&presage_candidates)
#endif
{
    // FIXME: Check whether spellchecker is enabled, and update enabled flag!
#ifdef HAVE_PRESAGE
    presage.config("Presage.Selector.SUGGESTIONS", "6");
    presage.config("Presage.Selector.REPEAT_SUGGESTIONS", "yes");
#endif
}


//! \param parent The owner of this instance. Can be 0, in case QObject
//!               ownership is not required.
WordEngine::WordEngine(QObject *parent)
    : QObject(parent)
    , d_ptr(new WordEnginePrivate)
{}


WordEngine::~WordEngine()
{}


//! \brief Returns whether the engine provides updates for word candidates.
bool WordEngine::isEnabled() const
{
    Q_D(const WordEngine);
    return d->enabled;
}


//! \brief Set whether the engine should provide updates for word candidates.
//! \param enabled Setting to true will be ignored if there's no word
//!                prediction or error correction backend available.
void WordEngine::setEnabled(bool enabled)
{
 // Don't allow to enable word engine if no backends are available:
#if defined(HAVE_PRESAGE) || defined(HAVE_HUNSPELL)
#else
    if (enabled) {
        qWarning() << __PRETTY_FUNCTION__
                   << "No backend available, cannot enable word engine!";
    }

    enabled = false;
#endif

    Q_D(WordEngine);

    if (d->enabled != enabled) {
        d->enabled = enabled;
        Q_EMIT enabledChanged(d->enabled);
    }
}


//! \brief Respond to changes in text model.
//! \param text The shared text model. Can trigger emission of
//!             \sa candidatesUpdated() if new candidates are available.
//!             Can update face of preedit in text.
void WordEngine::onTextChanged(const Model::SharedText &text)
{
#ifdef DISABLE_PREEDIT
    Q_UNUSED(text)
    return;
#else
    // FIXME: add possiblity to turn off the error correction for
    // entries that does not need it (like password entries).  Also,
    // with that we probably will want to turn off preedit styling at
    // all.

    if (text.isNull()) {
        qWarning() << __PRETTY_FUNCTION__
                   << "No text model specified.";
    }

    Q_D(WordEngine);

    if (not d->enabled) {
        return;
    }

    const QString &preedit(text->preedit());
    if (preedit.isEmpty()) {
        if (not d->candidates.isEmpty()) {
            d->candidates.clear();
            text->setPreeditFace(Model::Text::PreeditDefault);
            Q_EMIT candidatesUpdated(d->candidates);
        }
        return;
    }

    d->candidates.clear();

#ifdef HAVE_PRESAGE
    const QString &context = (text->surroundingLeft() + preedit);
    d->candidates_context = context.toStdString();
    const std::vector<std::string> predictions = d->presage.predict();

    // TODO: Fine-tune presage behaviour to also perform error correction, not just word prediction.
    if (not context.isEmpty()) {
        // FIXME: max_candidates should come from style, too:
        const static unsigned int max_candidates = 7;
        const int count(qMin<int>(predictions.size(), max_candidates));
        for (int index = 0; index < count; ++index) {
            const QString &prediction(QString::fromStdString(predictions.at(index)));

            // FIXME: don't show the word we typed as a candidate.
            if (d->candidates.contains(prediction)) {
                continue;
            }

            d->candidates.append(prediction);
        }
    }
#endif

    const bool correct_spelling(d->spell_checker.spell(preedit));

    if (d->candidates.isEmpty() and not correct_spelling) {
        d->candidates.append(d->spell_checker.suggest(preedit, 5));
    }

    text->setPreeditFace(d->candidates.isEmpty() ? (correct_spelling ? Model::Text::PreeditDefault
                                                                     : Model::Text::PreeditNoCandidates)
                                                 : Model::Text::PreeditActive);

    Q_EMIT candidatesUpdated(d->candidates);
#endif
}

}} // namespace Logic, MaliitKeyboard
