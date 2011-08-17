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

#include <QKeySequence>
#include <QDebug>

#include "mimkeymodel.h"

MImKeyBinding::MImKeyBinding()
    : keyAction(ActionInsert),
      cycleSet(""),
      accents(""),
      accented_labels(""),
      secondary_label(""),
      keyLabel(""),
      dead(false),
      quickPick(false),
      compose(false),
      rtl(false)
{
}

MImKeyBinding::MImKeyBinding(const QString &label)
    : keyAction(ActionInsert),
      keyLabel(label),
      dead(false),
      quickPick(false),
      compose(false),
      rtl(false)
{
}

QString MImKeyBinding::accented(QChar accent) const
{
    QString activeLabel;
    int index = accents.indexOf(accent);
    if (index >= 0 && index < accented_labels.length()) {
        activeLabel = accented_labels[index];
    } else {
        activeLabel = label();
    }

    return activeLabel;
}

QString MImKeyBinding::accentedLabels() const
{
    return accented_labels;
}

KeyEvent MImKeyBinding::toKeyEventImpl(QKeyEvent::Type eventType,
                                    Qt::KeyboardModifiers modifiers,
                                    const QString &labelText,
                                    bool isComposing) const
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
        text = "\r";
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
        specialKey = KeyEvent::ChangeSign;
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
    case ActionSwitch:
        specialKey = KeyEvent::Switch;
        break;
    case ActionOnOffToggle:
        specialKey = KeyEvent::OnOffToggle;
        break;
    case ActionCompose:
        if (isComposing) {
            specialKey = KeyEvent::Commit;
        } else {
            // compose key default acts as return key.
            key = Qt::Key_Return;
            text = "\r";
        }
        break;
    case NumActions:
        Q_ASSERT(false);
    }

    return KeyEvent(text, eventType, key, specialKey, modifiers);
}

KeyEvent MImKeyBinding::toKeyEvent(QKeyEvent::Type eventType, Qt::KeyboardModifiers modifiers) const
{
    return toKeyEventImpl(eventType, modifiers, label());
}

KeyEvent MImKeyBinding::toKeyEvent(QKeyEvent::Type eventType, QChar accent, Qt::KeyboardModifiers modifiers) const
{
    return toKeyEventImpl(eventType, modifiers, accented(accent));
}

KeyEvent MImKeyBinding::toKeyEvent(QKeyEvent::Type eventType, bool isComposing, Qt::KeyboardModifiers modifiers) const
{
    return toKeyEventImpl(eventType, modifiers, label(), isComposing);
}

MImKeyModel::MImKeyModel(MImKeyModel::StyleType style,
                         MImKeyModel::WidthType widthType,
                         bool isFixed,
                         bool isRtl,
                         const QString &id)
    : mStyle(style),
      mWidthType(widthType),
      isFixed(isFixed),
      isRtl(isRtl),
      keyId(id)
{
    bindings[NoShift] = 0;
    bindings[Shift] = 0;
    activeBindings[NoShift] = 0;
    activeBindings[Shift] = 0;
}

MImKeyModel::~MImKeyModel()
{
    if (bindings[NoShift] != bindings[Shift]) {
        delete bindings[NoShift];
    }
    delete bindings[Shift];

    // No need to delete activeBindings; they're only aliases
}

void MImKeyModel::setBinding(const MImKeyBinding &binding, bool shift)
{
    const MImKeyBinding *&store(bindings[shift ? Shift : NoShift]);
    if (store) {
        delete store;
    }
    store = &binding;
    activeBindings[shift ? Shift : NoShift] = &binding;
}

KeyEvent MImKeyModel::toKeyEvent(QKeyEvent::Type eventType, bool shift) const
{
    Qt::KeyboardModifiers modifiers(shift ? Qt::ShiftModifier : Qt::NoModifier);
    return binding(shift)->toKeyEvent(eventType, modifiers);
}

KeyEvent MImKeyModel::toKeyEvent(QKeyEvent::Type eventType, QChar accent, bool shift) const
{
    return binding(shift)->toKeyEvent(eventType, accent, shift ? Qt::ShiftModifier : Qt::NoModifier);
}

KeyEvent MImKeyModel::toKeyEvent(QKeyEvent::Type eventType, bool shift, bool isComposing) const
{
    Qt::KeyboardModifiers modifiers(shift ? Qt::ShiftModifier : Qt::NoModifier);
    return binding(shift)->toKeyEvent(eventType, isComposing, modifiers);
}


MImKeyModel::StyleType MImKeyModel::style() const
{
    return mStyle;
}

MImKeyModel::WidthType MImKeyModel::width() const
{
    return mWidthType;
}

bool MImKeyModel::isFixedWidth() const
{
    return isFixed;
}

bool MImKeyModel::rtl() const
{
    return isRtl;
}

QString MImKeyModel::id() const
{
    return keyId;
}

void MImKeyModel::overrideBinding(const MImKeyBinding *binding, bool shift)
{
    if(binding == 0)
    {
        binding = bindings[shift ? Shift : NoShift]; // reset to default
    }
    activeBindings[shift ? Shift : NoShift] = binding;
}
