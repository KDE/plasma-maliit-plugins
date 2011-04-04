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

