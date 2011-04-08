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



#ifndef FIXTURE_VIRTUALKEYBOARD_H
#define FIXTURE_VIRTUALKEYBOARD_H

class MImAbstractKey;
class MImKeyArea;

#include "mimkeymodel.h"
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
     */
    virtual const MImAbstractKey *getKey(const MImKeyArea * const widget,
                                         const QString &label) const;
    /*!
     *  \brief Get the IkeyButton (button details) by providing the action associated  by  a keyboard button
     *  \param widget the button area widget that is queried for the button of a given action.
     *  \param action  Action associated with a keyboard button
     */
    virtual const MImAbstractKey *getKey(const MImKeyArea * widget,
                                         const MImKeyBinding::KeyAction action) const;

    /*!
     *  \brief Get the value associated with the Attribute key of a keyboard button by providing the  action associated  by  the button
     *  \param widget the button area widget that is queried for the keyboard button of a given label
     *  \param label  button label of the virtualkeyboard
     *  \param attribute Attribute Key of the keyboard button which is queuried for
     *  \return QString  Value of Attribute of the keyboard button with the associated action
     */
    virtual QString getAttribute(const MImKeyArea * const widget,
                                 const QString &label,
                                 const QString &attribute);
    /*!
     *  \brief Get the value associated with the Attribute key of a keyboard button by providing the  action associated  by  the button
     *  \param widget the button area widget that is queried for the keyboard button of a given label
     *  \param action  Action associated with a keyboard button
     *  \param attribute Attribute Key of the keyboard button which is queuried for
     *  \return QString  Value of Attribute of the keyboard button with the associated action
     */
    virtual QString getAttribute(const MImKeyArea * const widget,
                                 const MImKeyBinding::KeyAction action,
                                 const QString &attribute);
};

#endif

