/* * This file is part of meego-keyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */



#ifndef ENGINEHANDLER_H
#define ENGINEHANDLER_H

#include <QObject>
#include <QMap>
#include <QPointer>
#include <QStringList>

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
     * \brief Returns true if this language supports context.
     */
    virtual bool hasContext() const = 0;

    /*!
     * \brief Returns true if this language keeps preedit when reseting.
     */
    virtual bool keepPreeditWhenReset() const = 0;

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
     * \brief This method will be called when keyboard host reset and keepPreeditWhenReset returns false.
     */
    virtual void resetPreeditWithoutCommit() = 0;

    /*!
     * \brief This method will be called when keyboard host reset and keepPreeditWhenReset returns true.
     */
    virtual void resetPreeditWithCommit() = 0;

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
     * \return "true" if the engine handler consumes the key event;
     *         Otherwise return "false".
     */
    virtual bool handleKeyClick(const KeyEvent &event) = 0;
};


#endif
