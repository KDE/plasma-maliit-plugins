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

    virtual bool handleKeyPress(const KeyEvent &event) = 0;

    virtual bool handleKeyRelease(const KeyEvent &event) = 0;

    virtual bool handleKeyClick(const KeyEvent &event) = 0;
};


#endif
