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



#include "singlewidgetbutton.h"
#include "singlewidgetbuttonarea.h"
#include "mvirtualkeyboardstyle.h"

#include <MTheme>
#include <QGraphicsItem>
#include <QPainter>

SingleWidgetButton::SingleWidgetButton(const VKBDataKey &key,
                                       const MVirtualKeyboardStyleContainer &style,
                                       QGraphicsItem &parent)
    : width(0),
      dataKey(key),
      shift(false),
      currentLabel(dataKey.binding(false)->label()),
      currentState(Normal),
      selected(false),
      styleContainer(style),
      parentItem(parent)
{
    icons[0] = (dataKey.binding(false) ? loadIcon(dataKey.binding(false)->action(), false) : 0);
    icons[1] = (dataKey.binding(true) ? loadIcon(dataKey.binding(true)->action(), true) : 0);
}

SingleWidgetButton::~SingleWidgetButton()
{
    if (icons[0]) {
        MTheme::releasePixmap(icons[0]);
    }
    if (icons[1]) {
        MTheme::releasePixmap(icons[1]);
    }
}

const QString SingleWidgetButton::label() const
{
    return currentLabel;
}

const QString SingleWidgetButton::secondaryLabel() const
{
    return binding().secondaryLabel();
}

QRect SingleWidgetButton::buttonRect() const
{
    return cachedButtonRect;
}

QRect SingleWidgetButton::buttonBoundingRect() const
{
    return cachedBoundingRect;
}

void SingleWidgetButton::setModifiers(bool shift, QChar accent)
{
    if (this->shift != shift || this->accent != accent) {
        this->shift = shift;
        this->accent = accent;
        currentLabel = binding().accented(accent);

        update();
    }
}

void SingleWidgetButton::setDownState(bool down)
{
    ButtonState newState;

    if (down) {
        // Pressed state is the same for selectable and non-selectable.
        newState = Pressed;
    } else {
        newState = (selected ? Selected : Normal);
    }

    if (newState != currentState) {
        currentState = newState;
        update();
    }
}

void SingleWidgetButton::setSelected(bool select)
{
    if (selected != select) {
        selected = select;

        // refresh state
        setDownState(currentState == Pressed);
    }
}

SingleWidgetButton::ButtonState SingleWidgetButton::state() const
{
    return currentState;
}

const VKBDataKey &SingleWidgetButton::key() const
{
    return dataKey;
}

const KeyBinding &SingleWidgetButton::binding() const
{
    return *dataKey.binding(shift);
}

bool SingleWidgetButton::isDeadKey() const
{
    return binding().isDead();
}

const QPixmap *SingleWidgetButton::icon() const
{
    return (shift ? icons[1] : icons[0]);
}

void SingleWidgetButton::drawIcon(const QRect &rectangle, QPainter *painter) const
{
    const QPixmap *iconPixmap = icon();
    if (iconPixmap) {
        QPointF iconPos(rectangle.x() + (rectangle.width() - iconPixmap->width()) / 2,
                        rectangle.y() + (rectangle.height() - iconPixmap->height()) / 2);
        painter->drawPixmap(iconPos, *iconPixmap);
    }
}

void SingleWidgetButton::update()
{
    // Invalidate this button's area.
    parentItem.update(buttonRect());
}

const QPixmap *SingleWidgetButton::loadIcon(KeyBinding::KeyAction action, const bool shift) const
{
    const QPixmap *pixmap = 0;
    const QString *id = 0;
    QSize size;

    switch(action) {
        case KeyBinding::ActionBackspace:
            id = &styleContainer->keyBackspaceIconId();
            size = styleContainer->keyBackspaceIconSize();
            break;
        case KeyBinding::ActionShift:
            if (shift) {
                id = &styleContainer->keyShiftUppercaseIconId();
            } else {
                id = &styleContainer->keyShiftIconId();
            }
            size = styleContainer->keyShiftIconSize();
            break;
        case KeyBinding::ActionReturn:
            id = &styleContainer->keyEnterIconId();
            size = styleContainer->keyEnterIconSize();
            break;
        case KeyBinding::ActionLayoutMenu:
            id = &styleContainer->keyMenuIconId();
            size = styleContainer->keyMenuIconSize();
            break;
        default:
            break;
    }

    if (id) {
        pixmap = MTheme::pixmap(*id, size);
    }

    return pixmap;
}
