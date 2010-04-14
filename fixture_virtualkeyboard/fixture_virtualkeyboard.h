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



#ifndef FIXTURE_VIRTUALKEYBOARD_H
#define FIXTURE_VIRTUALKEYBOARD_H

class IKeyButton;
class SingleWidgetButtonArea;

#include "vkbdatakey.h"
#include "tasfixtureplugininterface.h"

#include <QObject>

class FixtureVirtualKeyboard : public QObject, public TasFixturePluginInterface
{
  Q_OBJECT
  Q_INTERFACES(TasFixturePluginInterface)

public:
     explicit FixtureVirtualKeyboard(QObject *parent = 0);
     virtual ~FixtureVirtualKeyboard();

     bool execute(void *objectInstance,
                  const QString &actionName,
                  const QHash<QString, QString> &parameters,
                  QString &stdOut);

    /*!
     *  \brief Get the IkeyButton (button details) by providing the  label of the keyboard button
     *  \param widget the button area widget that is queried for the button of a given label.
     *  \param label  button label of the virtualkeyboard
     */
    virtual const IKeyButton *getKey(const SingleWidgetButtonArea * const widget,
                                     const QString &label) const;
    /*!
     *  \brief Get the IkeyButton (button details) by providing the action associated  by  a keyboard button
     *  \param widget the button area widget that is queried for the button of a given action.
     *  \param action  Action associated with a keyboard button
     */
    virtual const IKeyButton *getKey(const SingleWidgetButtonArea * widget,
                                     const KeyBinding::KeyAction action) const;
};

#endif

