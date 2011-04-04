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
