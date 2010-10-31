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

    if (actionName == "getPosition") {
        if (!parameters.contains("key") || parameters.value("key").isEmpty()) {
            stdOut = "action can be called only with valid argument: key ";
            return false;
        }
    }

    const MImAbstractKey *button = 0;
    const QString key = QString(parameters.value("key"));

    if (key == "Backspace") {
        button = getKey(widget, MImKeyBinding::ActionBackspace);
    } else if (key == "Shift") {
        button = getKey(widget, MImKeyBinding::ActionShift);
    } else if (key == "Sym") {
        button = getKey(widget, MImKeyBinding::ActionSym);
    } else if (key == "+/-") {
        button = getKey(widget, MImKeyBinding::ActionPlusMinusToggle);
    } else if (key == "*+") {
        button = getKey(widget, MImKeyBinding::ActionCycle);
    } else if (key == "Space") {
        button = getKey(widget, MImKeyBinding::ActionSpace);
    } else if (key == "Enter") {
        button = getKey(widget, MImKeyBinding::ActionReturn);
    } else if (key == "AccentKeys") {
        button = getKey(widget, MImKeyBinding::ActionSwitch);
    } else {
        button = getKey(widget, key);
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

    stdOut = "Could not get a proper button ";
    return false;
}

const MImAbstractKey * FixtureVirtualKeyboard::getKey(const MImKeyArea * const widget,
                                                  const QString &label) const
{
    Q_ASSERT(widget);

    foreach (const MImKeyArea::ButtonRow &row, widget->rowList) {
        foreach (const MImKey *button, row.buttons) {
            if (button->label() == label) {
                return button;
            }
        }
    }

    return 0;
}

const MImAbstractKey * FixtureVirtualKeyboard::getKey(const MImKeyArea * const widget,
                                                  MImKeyBinding::KeyAction action) const
{
    Q_ASSERT(widget);

    foreach (const MImKeyArea::ButtonRow &row, widget->rowList) {
        foreach (const MImKey *button, row.buttons) {
            if (button->binding().action() == action) {
                return button;
            }
        }
    }

    return 0;
}
