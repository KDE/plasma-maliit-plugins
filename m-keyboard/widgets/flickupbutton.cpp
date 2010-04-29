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



#include "flickupbutton.h"
#include "flickupbuttonview.h"
#include "vkbdatakey.h"
#include <MTheme>
#include <QDebug>

namespace
{
    const int FlickThreshold = 50;

    const QString ObjectNameBackspace("Backspace");
    const QString ObjectNameShiftNormal("ShiftNormal");
    const QString ObjectNameShiftLatched("ShiftLatched");
    const QString ObjectNameShiftLocked("ShiftLocked");
    const QString ObjectNameSpacebar("Spacebar");
    const QString ObjectNameSym("Sym");
    const QString ObjectNameLayoutMenu("LayoutMenu");
    const QString ObjectNameReturn("Enter");
}

FlickUpButton::FlickUpButton(const VKBDataKey &dataKey, QGraphicsItem *parent)
    : MButton(parent),
      flickEnabled(true),
      dataKey(dataKey),
      shift(false)
{
    FlickUpButtonView *view(new FlickUpButtonView(this));
    setView(view);

    updateButtonText();
    updateButtonObjectName();
}


FlickUpButton::~FlickUpButton()
{
}


bool FlickUpButton::onMove(QPointF pos)
{
    QRectF rect = boundingRect();
    bool res = false;

    if (pos.y() <= (rect.top() - FlickThreshold)) {
        flick();
        res = true;
    }

    return res;
}


void FlickUpButton::flick()
{
    if (getFlickEnabled()) {
        emit flicked();
    }
}


bool FlickUpButton::getFlickEnabled() const
{
    return flickEnabled;
}


void FlickUpButton::disableFlick()
{
    flickEnabled = false;
}


void FlickUpButton::enableFlick()
{
    flickEnabled = true;
}

void FlickUpButton::updateButtonText()
{
    setText(binding().accented(accent));
}

void FlickUpButton::updateButtonObjectName()
{
    QString objectName;

    switch (binding().action()) {
    case KeyBinding::ActionShift:
        objectName = ObjectNameShiftNormal;
        break;

    case KeyBinding::ActionSpace:
        objectName = ObjectNameSpacebar;
        break;

    case KeyBinding::ActionBackspace:
        objectName = ObjectNameBackspace;
        break;

    case KeyBinding::ActionSym:
        objectName = ObjectNameSym;
        break;

    case KeyBinding::ActionLayoutMenu:
        objectName = ObjectNameLayoutMenu;
        break;

    case KeyBinding::ActionReturn:
        objectName = ObjectNameReturn;
        break;

    case KeyBinding::ActionCycle:
    case KeyBinding::ActionInsert:
    case KeyBinding::ActionDecimalSeparator:
    case KeyBinding::ActionPlusMinusToggle:
        break;
    case KeyBinding::NumActions:
        Q_ASSERT(false);
        break;
    }

    if (this->objectName() != objectName) {
        setObjectName(objectName);
    }
}

// Overloaded methods from IKeyButton

const QString FlickUpButton::label() const
{
    return text();
}

const QString FlickUpButton::secondaryLabel() const
{
    return binding().secondaryLabel();
}

QRect FlickUpButton::buttonRect() const
{
    return geometry().toRect();
}

QRect FlickUpButton::buttonBoundingRect() const
{
    // Translate bounding rect to parent coordinates
    return QRectF(pos() + boundingRect().topLeft(),
                  boundingRect().size()).toRect();
}

void FlickUpButton::setModifiers(bool shift, QChar accent)
{
    bool updateLabel = false;
    bool updateStyle = false;

    if (this->shift != shift) {
        this->shift = shift;
        updateLabel = true;
        updateStyle = true;
    }
    if (this->accent != accent) {
        this->accent = accent;
        updateLabel = true;
    }

    if (updateLabel) {
        updateButtonText();
    }
    if (updateStyle) {
        updateButtonObjectName();
    }
}

void FlickUpButton::setSelected(bool select)
{
    setCheckable(select);
    if (select) {
        setChecked(true);
    }
}

void FlickUpButton::setDownState(bool down)
{
    // Update MButton down state.
    setDown(down);
}

IKeyButton::ButtonState FlickUpButton::state() const
{
    IKeyButton::ButtonState currentState;

    if (isDown()) {
        currentState = IKeyButton::Pressed;
    } else {
        currentState = isCheckable() ? IKeyButton::Selected : IKeyButton::Normal;
    }

    return currentState;
}

const VKBDataKey &FlickUpButton::key() const
{
    return dataKey;
}

const KeyBinding &FlickUpButton::binding() const
{
    return *dataKey.binding(shift);
}

bool FlickUpButton::isDeadKey() const
{
    return binding().isDead();
}
