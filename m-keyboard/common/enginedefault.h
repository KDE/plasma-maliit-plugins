/* * This file is part of m-keyboard *
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

#ifndef ENGINEDEFAULT_H
#define ENGINEDEFAULT_H

#include "abstractengine.h"

class MAbstractInputMethodHost;
class MImEngineWordsInterface;
class MGConfItem;

/*!
  \class EngineDefault

  \brief The EngineDefault class creates, initializes and keeps the MImEngineWordsInterface
   for the default input method engine.
*/
class EngineDefault : public AbstractEngine
{
    Q_OBJECT
    Q_DISABLE_COPY(EngineDefault)

public:
    EngineDefault(MAbstractInputMethodHost &imHost, const QString &engineName);
    virtual ~EngineDefault();

    /*!
     *\brief Returns the supported language list.
     */
    static QStringList supportedLanguages();

    //! reimp
    virtual MImEngineWordsInterface *engine() const;
    //! reimp_end

public slots:
    //! This slot is called when virtual keyboard language is changed.
    virtual void updateEngineLanguage(const QString &language);

private slots:
    //! Synchronize correction setting
    void synchronizeCorrectionSetting();

    //! Synchronize next word prediction setting
    void synchronizeNextWordPrediction();

protected:
    void initializeEngine();

private:
    MAbstractInputMethodHost &inputMethodHost;

    MImEngineWordsInterface *mEngine;
    //! default input method error correction setting
    MGConfItem *settingCorrection;
    MGConfItem *settingCorrectionSpace;
    MGConfItem *settingNextWordPrediction;
    MGConfItem *settingEngineName;

    QString engineLanguage;

    friend class Ut_EngineDefault;
};

#endif
