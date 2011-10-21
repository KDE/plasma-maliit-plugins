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


#ifndef ENGINEHANDLERTONAL_H
#define ENGINEHANDLERTONAL_H

#include "enginehandlerdefault.h"

class EngineHandlerTonal : public EngineHandlerDefault
{
public:
    EngineHandlerTonal(MKeyboardHost &keyboardHost);

    virtual ~EngineHandlerTonal();

    static QStringList supportedLanguages() {
        QStringList languages;
        languages << "vi" << "th";
        return languages;
    }

    //! \reimp
    virtual bool handleKeyPress(const KeyEvent &event);
    virtual bool handleKeyRelease(const KeyEvent &event);
    virtual bool handleKeyClick(const KeyEvent &event, bool cycleKeyActive);
    virtual void deactivate();
    virtual void clearPreedit(bool commit);
    virtual void editingInterrupted();
    virtual void resetHandler();
    virtual void preparePluginSwitching();
    //! \reimp_end

private: // functions

    /*! Figures out where to place the given Vietnamese tone in the given syllable.
     * \param context The syllable/word to place the tone in
     * \param cursorPos The current cursor position within the word
     * \param tone The tone to be placed
     * \return true if tone mark placed, false if e.g. there is no vowel
    */
    bool placeVietnameseTone(QString& context, int cursorPos, QChar& tone);

    /*! Determines whether the given input is legal to form a combined sequence in Thai.
     * \param prevLetter The letter preceding the input
     * \param curLetter The letter just input
     * \return true if the newly input letter should be accepted
    */
    bool isThaiInputAcceptable(QChar prevLetter, QChar curLetter);

private: // data

    MImEngineWordsInterface *engine;
    QChar autoPositionedTone;
};
#endif
