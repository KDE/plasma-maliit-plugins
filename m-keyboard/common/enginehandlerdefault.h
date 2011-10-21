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


#ifndef ENGINEHANDLERDEFAULT_H
#define ENGINEHANDLERDEFAULT_H

class EngineHandlerDefault : public EngineHandler
{
public:
    EngineHandlerDefault(MKeyboardHost &keyboardHost);

    virtual ~EngineHandlerDefault();
    
    static QStringList supportedLanguages() {
        return QStringList();
    }

    //! \reimp
    virtual void activate();
    virtual void deactivate();
    virtual AbstractEngineWidgetHost *engineWidgetHost();
    virtual bool cursorCanMoveInsidePreedit() const;
    virtual bool hasHwKeyboardIndicator() const;
    virtual bool hasErrorCorrection() const;
    virtual bool acceptPreeditInjection() const;
    virtual bool hasAutoCaps() const;
    virtual QList<QRegExp> autoCapsTriggers() const;
    virtual bool hasContext() const;
    virtual bool commitPreeditWhenInterrupted() const;
    virtual bool correctionAcceptedWithSpaceEnabled() const;
    virtual bool isComposingInputMethod() const;
    virtual bool supportTouchPointAccuracy() const;
    virtual bool commitWhenCandidateClicked() const;
    virtual void clearPreedit(bool commit);
    virtual void editingInterrupted();
    virtual void resetHandler();
    virtual void preparePluginSwitching();
    virtual bool handleKeyPress(const KeyEvent &event);
    virtual bool handleKeyRelease(const KeyEvent &event);
    virtual bool handleKeyClick(const KeyEvent &event, bool cycleKeyActive);
    virtual bool handleKeyCancel(const KeyEvent &event);
    //! \reimp_end

protected:
    MKeyboardHost &mKeyboardHost;

private:
    AbstractEngineWidgetHost *mEngineWidgetHost;
    QList<QRegExp> autoCapsTriggerList;
};

class EngineHandlerEnglish : public EngineHandlerDefault
{
public:
    EngineHandlerEnglish(MKeyboardHost &keyboardHost);

    virtual ~EngineHandlerEnglish();

    static QStringList supportedLanguages() {
        QStringList languages;
        languages << "en_gb" << "en_us";
        return languages;
    }

    //! \reimp
    virtual QList<QRegExp> autoCapsTriggers() const;
    //! \reimp_end

private:
    QList<QRegExp> autoCapsTriggerList;
};

#endif
