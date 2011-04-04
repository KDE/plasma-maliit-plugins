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

#ifndef ABSTRACTENGINE_H
#define ABSTRACTENGINE_H

#include <QObject>
#include <QString>
#include <QStringList>

class MAbstractInputMethodHost;
class MImEngineWordsInterface;
class MGConfItem;

/*!
  \class AbstractEngine

  \brief The AbstractEngine class creates, initializes and keeps the MImEngineWordsInterface
   for the default input method engine.
*/
class AbstractEngine : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractEngine)

public:
    AbstractEngine(MAbstractInputMethodHost &, const QString &) {};

    /*!
     *\brief Returns the supported language list.
     */
    static QStringList supportedLanguages() {return QStringList();};
    
    /*!
     * \brief Returns the pointer to error correction/prediction engine.
     */
    virtual MImEngineWordsInterface *engine() const = 0;

public slots:
    //! This slot is called when virtual keyboard language is changed.
    virtual void updateEngineLanguage(const QString &language) = 0;

signals:
    //! This signal is emitted when the correction setting is changed
    void correctionSettingChanged();

private:
    friend class Ut_AbstractEngine;
};

#endif
