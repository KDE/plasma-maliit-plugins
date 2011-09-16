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



#include "fixture_virtualkeyboard.h"
#include "mimkeyarea.h"
#include "mimkey.h"
#include "mimabstractkey.h"
#include "mimkeyarea_p.h"

#include <QDebug>
#include <QString>
#include <QHash>

Q_EXPORT_PLUGIN2(fixture_virtualkeyboard, FixtureVirtualKeyboard)

FixtureVirtualKeyboard::FixtureVirtualKeyboard(QObject *parent)
    : QObject(parent)
{
}

FixtureVirtualKeyboard::~FixtureVirtualKeyboard()
{
}
/*!
 *  \brief execute is called when implementing a fixture from ruby matti
 */
bool FixtureVirtualKeyboard::execute(void *objectInstance,
                                     const QString &actionName,
                                     const QHash<QString, QString> &parameters,
                                     QString &stdOut)
{
    qDebug() << "FixtureVirtualKeyboard::execute action:" << actionName;

    if (objectInstance == 0) {
        stdOut = "This fixture can be called for MImAbstractKeyArea only!";
        return false;
    }

    QGraphicsItem *gItem = (QGraphicsItem*)objectInstance;
    const MImKeyArea *const widget =  qgraphicsitem_cast<MImKeyArea *>(gItem);

    if (!widget) {
        stdOut = "This fixture can be called for MImAbstractKeyArea only!";
        return false;
    }

    if (!parameters.contains("key") || parameters.value("key").isEmpty()) {
        stdOut = "action can be called only with valid argument: key ";
        return false;
    }

    const MImAbstractKey *button = 0;
    const QString key = QString(parameters.value("key"));

    MImKeyBinding::KeyAction action;
    bool keyAction = true;
    if (key == "Backspace") {
        action = MImKeyBinding::ActionBackspace;
    } else if (key == "Shift") {
        action = MImKeyBinding::ActionShift;
    } else if (key == "Sym") {
        action = MImKeyBinding::ActionSym;
    } else if (key == "+/-") {
        action = MImKeyBinding::ActionPlusMinusToggle;
    } else if (key == "*+") {
        action = MImKeyBinding::ActionCycle;
    } else if (key == "Space") {
        action = MImKeyBinding::ActionSpace;
    } else if (key == "Enter") {
        action = MImKeyBinding::ActionReturn;
    } else if (key == "Compose") {
        action = MImKeyBinding::ActionCompose;
    } else if (key == "AccentKeys") {
        action = MImKeyBinding::ActionSwitch;
    } else if (key == "OnOffToggle") {
        action = MImKeyBinding::ActionOnOffToggle;
    } else {
        keyAction = false;
    }

    if (actionName == "getPosition") {
            if (!keyAction){
                button = getKey(widget, key);
            } else {
                button = getKey(widget, action);
            }
            if (button) {
                const QRect rect = button->buttonRect().toRect();
                stdOut = QString("x=%1,y=%2,width=%3,height=%4")
                    .arg(rect.center().x())
                    .arg(rect.center().y())
                    .arg(rect.width())
                    .arg(rect.height());
                return true;
            }
    } else if (actionName == "getAttribute") {
        if (!parameters.contains("attribute") || parameters.value("attribute").isEmpty()) {
            stdOut = "attribute can be called only with valid argument: attribute ";
            return false;
        }
        const QString attribute = QString(parameters.value("attribute"));
        if (!keyAction){
            stdOut = getAttribute(widget, key, attribute);
        } else {
            stdOut = getAttribute(widget, action, attribute);
        }
        return true;
    }

    stdOut = "Could not get a proper button " + key;
    return false;
}

const MImAbstractKey * FixtureVirtualKeyboard::getKey(const MImKeyArea * const widget,
                                                      const QString &label) const
{
    Q_ASSERT(widget);

    foreach (const MImKeyAreaPrivate::KeyRow &row, widget->d_ptr->rowList) {
        foreach (const MImKey *key, row.keys) {
            if (key->label() == label) {
                return key;
            }
        }
    }

    return 0;
}

const MImAbstractKey * FixtureVirtualKeyboard::getKey(const MImKeyArea * const widget,
                                                      MImKeyBinding::KeyAction action) const
{
    Q_ASSERT(widget);

    foreach (const MImKeyAreaPrivate::KeyRow &row, widget->d_ptr->rowList) {
        foreach (const MImKey *key, row.keys) {
            if (key->binding().action() == action) {
                return key;
            }
        }
    }

    return 0;
}

QString FixtureVirtualKeyboard::getAttribute(const MImKeyArea * const widget,
                                             MImKeyBinding::KeyAction action,
                                             const QString &attribute)
{
    Q_ASSERT(widget);
    QString output;

    foreach (const MImKeyAreaPrivate::KeyRow &row, widget->d_ptr->rowList) {
        foreach (const MImKey *key, row.keys) {
            if (key->binding().action() == action) {
                if ( attribute == "label" ){
                    output = key->label();
                }
                break;
            }
        }
    }

    return output;
}

QString FixtureVirtualKeyboard::getAttribute(const MImKeyArea * const widget,
                                             const QString &label,
                                             const QString &attribute)
{
    Q_ASSERT(widget);
    QString output;

    foreach (const MImKeyAreaPrivate::KeyRow &row, widget->d_ptr->rowList) {
        foreach (const MImKey *key, row.keys) {
            if (key->label() == label) {
                if ( attribute == "label" ){
                    output = key->label();
                }
                break;
            }
        }
    }

    return output;
}

