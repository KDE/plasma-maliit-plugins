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

#ifndef ENGINECJK
#define ENGINECJK

#include "abstractengine.h"

class MAbstractInputMethodHost;
class MVirtualKeyboard;
class MImEngineWordsInterface;

/*!
  \class EngineCJK

  \brief The EngineCJK class creates, initializes and keeps the MImEngineWordsInterface
   for the Chinese, or Japanese or Korea input method engine.
*/
class EngineCJK : public AbstractEngine
{
    Q_OBJECT
    Q_DISABLE_COPY(EngineCJK)

public:
    EngineCJK(MAbstractInputMethodHost &imHost, const QString &engineName);

    virtual ~EngineCJK();

    /*!
     *\brief Returns the supported language list.
     */
    static QStringList supportedLanguages();

    //! reimp
    virtual MImEngineWordsInterface *engine() const;
    virtual bool correctionAcceptedWithSpaceEnabled() const;
    //! reimp_end

public slots:
    //! reimp
    virtual void updateEngineLanguage(const QString &language);
    //! reimp_end

protected slots:
    void synchronizeFuzzy();

    void synchronizeWordPrediction();

    void synchronizeScriptPriority();

private:
    void initializeEngine();

    MAbstractInputMethodHost &inputMethodHost;

    MImEngineWordsInterface *mEngine;

    MGConfItem *settingFuzzy;
    MGConfItem *settingWordPrediction;
    MGConfItem *settingScriptPriority;

    friend class Ut_MKeyboarEngineManager;
};

#endif
