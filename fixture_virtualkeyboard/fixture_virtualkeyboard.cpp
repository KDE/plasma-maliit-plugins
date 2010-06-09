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
#include "singlewidgetbuttonarea.h"
#include "singlewidgetbutton.h"
#include "ikeybutton.h"

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

bool FixtureVirtualKeyboard::execute(void *objectInstance, 
                                     const QString &actionName,
                                     const QHash<QString, QString> &parameters,
                                     QString &stdOut)
{
    qDebug() << "FixtureVirtualKeyboard::execute action:" << actionName;

    if (objectInstance == 0) {
        stdOut = "This fixture can be called for KeyButtonArea only!";
        return false;
    }

    QGraphicsItem *gItem = (QGraphicsItem*)objectInstance;
    const SingleWidgetButtonArea *const widget =  qgraphicsitem_cast<SingleWidgetButtonArea *>(gItem);

    if (!widget) {
        stdOut = "This fixture can be called for Keybuttonarea only!";
        return false;
    }

    if (actionName == "getPosition") {
        if (!parameters.contains("key") || parameters.value("key").isEmpty()) {
            stdOut = "action can be called only with valid argument: key ";
            return false;
        }
    }

    const IKeyButton *button = 0;
    const QString key = QString(parameters.value("key"));

    if (key == "Backspace") {
        button = getKey(widget, KeyBinding::ActionBackspace);
    } else if (key == "Shift") {
        button = getKey(widget, KeyBinding::ActionShift);
    } else if (key == "Sym") {
        button = getKey(widget, KeyBinding::ActionSym);
    } else if (key == "+/-") {
        button = getKey(widget, KeyBinding::ActionPlusMinusToggle);
    } else if (key == "*+") {
        button = getKey(widget, KeyBinding::ActionCycle);
    } else if (key == "Space") {
        button = getKey(widget, KeyBinding::ActionSpace);
    } else if (key == "Enter") {
        button = getKey(widget, KeyBinding::ActionReturn);
    } else {
        button = getKey(widget, key);
    }

    if (button) {
        const QRect rect = button->buttonRect();
        stdOut = QString("x=%1,y=%2,width=%3,height=%4")
            .arg(rect.center().x())
            .arg(rect.center().y())
            .arg(rect.width())
            .arg(rect.height());
        return true;
    }

    stdOut = "Could not get a proper button ";
    return false;
}

const IKeyButton * FixtureVirtualKeyboard::getKey(const SingleWidgetButtonArea * const widget,
                                                  const QString &label) const
{
    Q_ASSERT(widget);

    foreach (const SingleWidgetButtonArea::ButtonRow &row, widget->rowList) {
        foreach (const SingleWidgetButton *button, row.buttons) {
            if (button->label() == label) {
                return button;
            }
        }
    }

    return 0;
}

const IKeyButton * FixtureVirtualKeyboard::getKey(const SingleWidgetButtonArea * const widget,
                                                  KeyBinding::KeyAction action) const
{
    Q_ASSERT(widget);

    foreach (const SingleWidgetButtonArea::ButtonRow &row, widget->rowList) {
        foreach (const SingleWidgetButton *button, row.buttons) {
            if (button->binding().action() == action) {
                return button;
            }
        }
    }

    return 0;
}
