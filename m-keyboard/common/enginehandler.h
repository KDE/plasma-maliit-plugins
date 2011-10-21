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



#ifndef ENGINEHANDLER_H
#define ENGINEHANDLER_H

#include <QObject>
#include <QMap>
#include <QPointer>
#include <QStringList>
#include <QList>
#include <QRegExp>

class MAbstractKeyboardEngine;
class MImEngineWordsInterface;
class AbstractEngineWidgetHost;
class MKeyboardHost;
class KeyEvent;

/*!
 * \brief EngineHandler defines the UI properties for \a supportedLanguages().
 *
 * EngineHandler class contains properties of the UI for the \a supportedLanguages()
 * as well as handling functions for inputting text according to the UI design
 * of the \a supportedLanguages().
 */
class EngineHandler : public QObject
{
    Q_OBJECT
public:
    EngineHandler(MKeyboardHost &) {};
    virtual ~EngineHandler() {};
    
    /*!
     * \brief Returns the supported language list.
     */
    static QStringList supportedLanguages() {
        return QStringList();
    };

    /*!
     * \brief activate this language properties.
     */
    virtual void activate() = 0;

    /*!
     * \brief deactivate this language properties.
     */
    virtual void deactivate() = 0;

    /*!
     * \brief Returns the pointer of engineWidgetHost.
     */
    virtual AbstractEngineWidgetHost *engineWidgetHost() = 0;

    /*!
     * \brief Returns true if the cursor can be moved inside preedit.
     */
    virtual bool cursorCanMoveInsidePreedit() const = 0;

    /*!
     * \brief Returns true if this language has hardware keyboard indicator.
     */
    virtual bool hasHwKeyboardIndicator() const = 0;

    /*!
     * \brief Returns true if this language has error correction.
     */
    virtual bool hasErrorCorrection() const = 0;

    /*!
     * \brief Returns true if this language accepts preedit injection.
     */
    virtual bool acceptPreeditInjection() const = 0;

    /*!
     * \brief Returns true if this language supports auto capitalization.
     */
    virtual bool hasAutoCaps() const = 0;

    /*!
     * \brief Returns a list of regular expressions that match for auto capitalization cases.
     */
    virtual QList<QRegExp> autoCapsTriggers() const = 0;

    /*!
     * \brief Returns true if this language supports context.
     */
    virtual bool hasContext() const = 0;

    /*!
     * \brief Returns true if language would like to commit preedit when editing is interrupted.
     *
     * Typical case for interruption is for example when changing to other language or to other input method
     * plugin.
     */
    virtual bool commitPreeditWhenInterrupted() const = 0;

    /*!
     * \brief Returns true if this language accepts correction suggestion with space key.
     */
    virtual bool correctionAcceptedWithSpaceEnabled() const = 0;

    /*!
     * \brief Returns true if this language is the composing input method.
     */
    virtual bool isComposingInputMethod() const = 0;

    /*!
     * \brief Returns true if this language supports error correction/preediction with touch point accuracy.
     */
    virtual bool supportTouchPointAccuracy() const = 0;

    /*!
     * \brief Returns true if this language will commit candidate when clicking it.
     */
    virtual bool commitWhenCandidateClicked() const = 0;

    /*!
     * \brief Clears preedit from keyboard host and input method host.
     *
     * \param commit, indicates whether the preedit should be commited before clearing or not
     */
    virtual void clearPreedit(bool commit) = 0;
    /*!
     * \brief Called when editing has been interrupted.
     *
     * At this point engine handler should perform needed actions, for example commit and clear preedit.
     */
    virtual void editingInterrupted() = 0;

    /*!
     * \brief Called when keyboard host is reset.
     *
     * This reset is intended for reseting the handler only. Do not send anything to keyboard host or
     * input method host from this method.
     */
    virtual void resetHandler() = 0;

    /*!
     * \brief Prepare the plugin switching.
     */
    virtual void preparePluginSwitching() = 0;

    /*!
     * \brief Handle a key press event.
     * \param event, the key press event to be processed.
     * \return "true" if the engine handler consumes the key event;
     *         Otherwise return "false".
     */
    virtual bool handleKeyPress(const KeyEvent &event) = 0;

    /*!
     * \brief Handle a key release event.
     * \param event, the key release event to be processed.
     * \return "true" if the engine handler consumes the key event;
     *         Otherwise return "false".
     */
    virtual bool handleKeyRelease(const KeyEvent &event) = 0;

    /*!
     * \brief Handle a key click event.
     * Notes:
     * -# Generally, if the key click event represents a text input event, an engine handler will consume
     * it and will not broadcast it to other components. In this case, this method will return "true".
     * -# If the key click event represents a special function (for example, a "Sym" key), an engine handler
     * can not process it and will return "false".
     * -# An engine handler will update internal candidates after processing the key click event. However,
     * it can not complete refreshing other UI parts (for example, update Caps Lock state). So the remained
     * UI update should be done by the caller after this method returns.
     * \param event, the key click event to be processed.
     * \param cycleKeyActive, current state of cycle key handling in the keyboard host.
     * \return "true" if the engine handler consumes the key event;
     *         Otherwise return "false".
     */
    virtual bool handleKeyClick(const KeyEvent &event, bool cycleKeyActive = false) = 0;

    /*!
     * \brief Handle a key cacellation event.
     * \param event, the key cacellation event to be processed.
     * \return "true" if the engine handler consumes the key event;
     *         Otherwise return "false".
     */
    virtual bool handleKeyCancel(const KeyEvent &event) = 0;

};


#endif
