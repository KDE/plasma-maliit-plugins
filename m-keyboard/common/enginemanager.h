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



#ifndef ENGINEMANAGER_H
#define ENGINEMANAGER_H

#include "enginehandler.h"
#include <QObject>
#include <QMap>
#include <QPointer>

class AbstractEngine;
class MImEngineWordsInterface;
class MKeyboardHost;

/*!
 * \brief The EngineManager class manages the engine handlers and their engines.
 *
 * The EngineManager loads all the handlers and engines and provide them to
 * MKeyboardHost according to currently used language.
 * 
 */
class EngineManager : public QObject
{
    Q_OBJECT
public:
    virtual ~EngineManager();

    //! \brief Get singleton instance
    //! \return singleton instance
    static EngineManager &instance();

    //! \brief Create singleton
    static void createInstance(MKeyboardHost &);

    //! \brief Destroy singleton
    static void destroyInstance();

    /*!
     * \brief Returns the pointer of current used EngineHandler.
     */
    EngineHandler *handler() const;

    /*!
     * \brief Returns the pointer of current used error correction/prediction engine.
     */
    MImEngineWordsInterface *engine() const;

    /*!
     * \brief Returns current language.
     */
    QString activeLanguage() const;

    /*!
     * \brief Ensure recorded language is the same as actual engine language.
     */
    void ensureLanguageInUse(const QString &lang);

    /*!
     * \brief Returns recorded language is valid or not.
     */
    bool languageIsValid() const;

public slots:
    //! This slot is called when virtual keyboard language is changed.
    void updateLanguage(const QString &language);

signals:
    //! This signal is emitted when the correction setting is changed
    void correctionSettingChanged();

private:
    EngineManager(MKeyboardHost &);

    void init();

    EngineHandler *findOrCreateEngineHandler(const QString &language);

    AbstractEngine *findOrCreateEngine(const QString &language, const QString &engineName);

private:
    MKeyboardHost &mKeyboardHost;
    EngineHandler *currentEngineHandler;
    AbstractEngine *currentEngine;
    QString mLanguage;

    QMap<QString, QPointer<EngineHandler> > handlerMap;
    QMap<QString, QPointer<AbstractEngine> > engineMap;

    static EngineManager *Instance;
};

#endif
