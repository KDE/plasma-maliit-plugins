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

#include "vkbdatakey.h"
#include <QKeySequence>

KeyBinding::KeyBinding()
    : keyAction(ActionInsert),
      cycleSet(""),
      accents(""),
      accentedLabels(""),
      secondary_label(""),
      keyLabel(""),
      dead(false)
{
}

QString KeyBinding::accented(QChar accent) const
{
    QString activeLabel;
    int index = accents.indexOf(accent);
    if (index >= 0 && index < accentedLabels.length()) {
        activeLabel = accentedLabels[index];
    } else {
        activeLabel = label();
    }

    return activeLabel;
}

KeyEvent KeyBinding::toKeyEventImpl(QKeyEvent::Type eventType,
                                    Qt::KeyboardModifiers modifiers,
                                    const QString &labelText) const
{
    Qt::Key key = Qt::Key_unknown;
    KeyEvent::SpecialKey specialKey = KeyEvent::NotSpecial;
    QString text(labelText);

    switch (keyAction) {
    case ActionShift:
        key = Qt::Key_Shift;
        break;
    case ActionSpace:
        key = Qt::Key_Space;
        text = " ";
        break;
    case ActionBackspace:
        key = Qt::Key_Backspace;
        text = "\b";
        break;
    case ActionReturn:
        key = Qt::Key_Return;
        text = "\n";
        break;
    case ActionTab:
        key = Qt::Key_Tab;
        text = "\t";
        break;
    case ActionCycle:
        specialKey = KeyEvent::CycleSet;
        text = cycleSet;
        break;
    case ActionLayoutMenu:
        specialKey = KeyEvent::LayoutMenu;
        break;
    case ActionSym:
        specialKey = KeyEvent::Sym;
        break;
    case ActionPlusMinusToggle:
        key = Qt::Key_plusminus;
        break;
    case ActionDecimalSeparator:
    case ActionInsert:
        if (!text.isEmpty()) {
            key = static_cast<Qt::Key>(QKeySequence(text)[0]);
        }

        break;
    case ActionCommit:
        specialKey = KeyEvent::Commit;
        break;
    case NumActions:
        Q_ASSERT(false);
    }

    return KeyEvent(text, eventType, key, specialKey, modifiers);
}

KeyEvent KeyBinding::toKeyEvent(QKeyEvent::Type eventType, Qt::KeyboardModifiers modifiers) const
{
    return toKeyEventImpl(eventType, modifiers, label());
}

KeyEvent KeyBinding::toKeyEvent(QKeyEvent::Type eventType, QChar accent, Qt::KeyboardModifiers modifiers) const
{
    return toKeyEventImpl(eventType, modifiers, accented(accent));
}


VKBDataKey::VKBDataKey(VKBDataKey::Style style, VKBDataKey::SizeGroup size, bool isFixed, bool isRtl)
    : styleType(style),
      sizeGroupType(size),
      isFixed(isFixed),
      isRtl(isRtl)
{
    bindings[NoShift] = 0;
    bindings[Shift] = 0;
}

VKBDataKey::~VKBDataKey()
{
    if (bindings[NoShift] != bindings[Shift]) {
        delete bindings[NoShift];
    }
    delete bindings[Shift];
}

KeyEvent VKBDataKey::toKeyEvent(QKeyEvent::Type eventType, bool shift) const
{
    Qt::KeyboardModifiers modifiers(shift ? Qt::ShiftModifier : Qt::NoModifier);
    return binding(shift)->toKeyEvent(eventType, modifiers);
}

KeyEvent VKBDataKey::toKeyEvent(QKeyEvent::Type eventType, QChar accent, bool shift) const
{
    return binding(shift)->toKeyEvent(eventType, accent, shift ? Qt::ShiftModifier : Qt::NoModifier);
}

VKBDataKey::Style VKBDataKey::style() const
{
    return styleType;
}

VKBDataKey::SizeGroup VKBDataKey::sizeGroup() const
{
    return sizeGroupType;
}

bool VKBDataKey::fixed() const
{
    return isFixed;
}

bool VKBDataKey::rtl() const
{
    return isRtl;
}

