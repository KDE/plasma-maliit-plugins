/* * This file is part of dui-keyboard *
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



#include "duihardwarekeyboard.h"
#include "duivirtualkeyboard.h"
#include <QKeyEvent>
#include <QDebug>

namespace
{
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;
    const Qt::Key FnLevelKey = Qt::Key_AltGr;
    const Qt::Key SymKey = Qt::Key_Multi_key;
};

DuiHardwareKeyboard::ModifierKey::ModifierKey(Qt::KeyboardModifier m, ModifierState s)
    : modifier(m),
      state(s),
      inBetweenPressRelease(false)
{
}

DuiHardwareKeyboard::RedirectedKey::RedirectedKey(Qt::Key key, bool eatKey, bool eatSelf)
    : keyCode(key),
      eatInBetweenKeys(eatKey),
      eatItself(eatSelf),
      pressed(false),
      charKeyClicked(false),
      lastClickedCharacter(QChar()),
      charKeyClickedCount(0)
{
    const Qt::KeyboardModifier m = DuiHardwareKeyboard::keyToModifier(keyCode);
    if (m != Qt::NoModifier) {
        modifier = ModifierKey(m, ModifierClearState);
    }
}

void DuiHardwareKeyboard::RedirectedKey::reset()
{
    pressed = false;
    charKeyClicked = false;
    lastClickedCharacter = QChar();
    charKeyClickedCount = 0;
    if (modifier.modifier != Qt::NoModifier) {
        modifier.state = ModifierClearState;
        modifier.inBetweenPressRelease = false;
    }
}

DuiHardwareKeyboard::DuiHardwareKeyboard(QObject *parent)
    : QObject(parent),
      keyboardType(Dui::FreeTextContentType),
      autoCaps(false)
{
    init();
}


DuiHardwareKeyboard::~DuiHardwareKeyboard()
{
}

void DuiHardwareKeyboard::setKeyboardType(Dui::TextContentType type)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << type;
    keyboardType = type;
    switch (keyboardType) {
    case Dui::NumberContentType:
    case Dui::PhoneNumberContentType:
        // With number and phone number content type, FnLevelModifier must be permanently locked
        setModifierState(FnLevelModifier, ModifierLockedState);
        break;
    default:
        break;
    }
}

void DuiHardwareKeyboard::reset()
{
    qDebug() << __PRETTY_FUNCTION__;
    for (int i = 0; i < sensitiveKeys.count(); i++) {
        if (sensitiveKeys[i].modifier.modifier != Qt::NoModifier) {
            //call unlockModifier to ensure clean.
            duiXkb.unlockModifiers(sensitiveKeys[i].modifier.modifier);
        }
        sensitiveKeys[i].reset();
    }
    autoCaps = false;
    filterNextKey = false;
    emit modifierStateChanged(Qt::ShiftModifier, ModifierClearState);
}

void DuiHardwareKeyboard::init()
{
    sensitiveKeys.append(DuiHardwareKeyboard::RedirectedKey(Qt::Key_Shift, false, false));
    sensitiveKeys.append(DuiHardwareKeyboard::RedirectedKey(FnLevelKey, false, false));
    sensitiveKeys.append(DuiHardwareKeyboard::RedirectedKey(SymKey, true, false));
}

Qt::KeyboardModifier DuiHardwareKeyboard::keyToModifier(Qt::Key keyCode)
{
    switch (keyCode) {
    case Qt::Key_Shift:
        return Qt::ShiftModifier;
    case FnLevelKey:
        return FnLevelModifier;
    case Qt::Key_Control:
        return Qt::ControlModifier;
    case Qt::Key_Meta:
        return Qt::MetaModifier;
    case Qt::Key_Alt:
        return Qt::AltModifier;
    default:
        break;
    }
    return Qt::NoModifier;
}

int DuiHardwareKeyboard::redirectedKeyIndex(Qt::Key keyCode) const
{
    int matchedIndex = -1;
    for (int i = 0; i < sensitiveKeys.count(); i++) {
        if (sensitiveKeys[i].keyCode == keyCode) {
            matchedIndex = i;
            break;
        }
    }
    return matchedIndex;
}

int DuiHardwareKeyboard::redirectedKeyIndex(Qt::KeyboardModifier modifier) const
{
    int matchedIndex = -1;
    for (int i = 0; i < sensitiveKeys.count(); i++) {
        if (sensitiveKeys[i].modifier.modifier == modifier) {
            matchedIndex = i;
            break;
        }
    }
    return matchedIndex;
}

bool DuiHardwareKeyboard::isSensitiveKeyPressed() const
{
    foreach (const RedirectedKey &key, sensitiveKeys) {
        if (key.pressed) {
            return true;
        }
    }
    return false;
}

bool DuiHardwareKeyboard::filterKeyEvent(bool forceProcessing, QEvent::Type keyType, Qt::Key keyCode,
                                         Qt::KeyboardModifiers /* modifiers */, const QString &text,
                                         bool /* autoRepeat */, int /* count */, int /* nativeScanCode */)
{
    bool eaten = false;
    const bool pressed = (keyType == QEvent::KeyPress);
    const bool filterThisKey = filterNextKey;
    filterNextKey = false;

    int matchedIndex = redirectedKeyIndex(keyCode);
    if (matchedIndex >= 0) {
        //for redirected key input
        if (sensitiveKeys[matchedIndex].modifier.modifier != Qt::NoModifier) {
            //modifier key press and release
            ModifierKey &targetModifierKey = sensitiveKeys[matchedIndex].modifier;
            if (pressed) {
                modifierKeyPress(targetModifierKey);
            } else {
                modifierKeyRelease(targetModifierKey);
            }
        } else {
            //other sensitive key
            if (!pressed && keyCode == SymKey) {
                symbolKeyClick();
            }
        }
        //record key state
        sensitiveKeys[matchedIndex].pressed = pressed;
        sensitiveKeys[matchedIndex].charKeyClicked = false;
        sensitiveKeys[matchedIndex].lastClickedCharacter = QChar();
        sensitiveKeys[matchedIndex].charKeyClickedCount = 0;

        eaten = sensitiveKeys[matchedIndex].eatItself;
    } else if (filterThisKey || forceProcessing || isSensitiveKeyPressed()) {
        // other modifier key (e.g. Ctrl, Alt) should not unlatch the latched modifier
        // TODO: if the comment above is correct, shouldn't the code have requested the next
        // key to be redirected in case this one was redirected due to nextKeyRedirectedRequest?
        if (keyToModifier(keyCode) == Qt::NoModifier) {
            //incoming key event is not a registered "redirectedKey" (modifier key, or symbol key),
            //but a character key. And only handle the key events which is pressed.
            if (pressed) {
                characterKeyClick(keyCode, text);
            }
        }

        for (int i = 0; i < sensitiveKeys.count(); ++i) {
            if (sensitiveKeys[i].pressed && sensitiveKeys[i].eatInBetweenKeys) {
                eaten = true;
                break;
            }
        }
    }

    return eaten;
}

void DuiHardwareKeyboard::setModifierState(Qt::KeyboardModifier modifier,
        ModifierState targetState)
{
    int matchedIndex = redirectedKeyIndex(modifier);
    if (matchedIndex < 0)
        return;
    ModifierKey &targetModifierKey = sensitiveKeys[matchedIndex].modifier;
    setModifierState(targetModifierKey, targetState);
}

void DuiHardwareKeyboard::setModifierState(ModifierKey &modifierKey, ModifierState targetState)
{
    if (modifierKey.modifier == Qt::NoModifier
        || !isValidModifierState(modifierKey.modifier, targetState))
        return;
    Qt::KeyboardModifier modifier = modifierKey.modifier;
    if (targetState == modifierKey.state)
        return;

    switch (targetState) {
    case ModifierLatchedState:
    case ModifierLockedState:
        //use lockModifiers() for both ModifierLatchedState and ModifierLockedState,
        //we don't want hardware keyboard to change the modifier state by itself
        //(e.g pressing any after latching will unlatch the modifier)
        duiXkb.lockModifiers(modifier);
        break;
    case ModifierClearState:
        duiXkb.unlockModifiers(modifier);
        modifierKey.inBetweenPressRelease = false;
        break;
    }
    modifierKey.state = targetState;
    if (modifier == Qt::ShiftModifier)
        emit shiftLevelChanged();

    emit modifierStateChanged(modifier, targetState);
}

void DuiHardwareKeyboard::setAutoCapitalization(bool caps)
{
    if (autoCaps != caps) {
        //set auto caps lock/unlock when there is no custom state being set for shift modifier
        if ((modifierState(Qt::ShiftModifier) == ModifierClearState) || autoCaps) {
            setModifierState(Qt::ShiftModifier, (caps ? ModifierLatchedState : ModifierClearState));
            autoCaps = caps;
        }
    }
}

ModifierState DuiHardwareKeyboard::modifierState(Qt::KeyboardModifier modifier) const
{
    ModifierState state = ModifierClearState;
    int matchedIndex = redirectedKeyIndex(modifier);
    if (matchedIndex >= 0) {
        state = sensitiveKeys[matchedIndex].modifier.state;
    }
    return state;
}

void DuiHardwareKeyboard::modifierKeyPress(ModifierKey &targetModifierKey)
{
    if (targetModifierKey.modifier == Qt::NoModifier)
        return;

    //release the other latched/locked modifier key
    for (int i = 0; i < sensitiveKeys.count(); i++) {
        if ((sensitiveKeys[i].modifier.state != ModifierClearState)
            && (sensitiveKeys[i].modifier.modifier != targetModifierKey.modifier)) {
            setModifierState(sensitiveKeys[i].modifier, ModifierClearState);
        }
    }

    const ModifierState state = targetModifierKey.state;
    //modifier key press in clear state, clear -> latched,
    if (state == ModifierClearState)
        setModifierState(targetModifierKey, ModifierLatchedState);
}

void DuiHardwareKeyboard::modifierKeyRelease(ModifierKey &targetModifierKey)
{
    if (targetModifierKey.modifier == Qt::NoModifier) {
        return;
    }

    const ModifierState state = targetModifierKey.state;
    //modifier key release
    if (state != ModifierClearState) {
        //cycling: latched-> latched + inBetweenPressRelease -> locked -> clear
        ModifierState nextState = ModifierClearState;
        if (state == ModifierLockedState) {
            nextState = ModifierClearState;
        } else if (state == ModifierLatchedState) {
            if ((targetModifierKey.modifier == Qt::ShiftModifier) && autoCaps) {
                //if the latched state is turn on by autoCaps,
                //then this shift modifier input will change the state -> clear
                nextState = ModifierClearState;
                autoCaps = false;
            } else if (targetModifierKey.inBetweenPressRelease) {
                //latched -> locked
                nextState = ModifierLockedState;
            } else {
                //latched -> latched + inBetweenPressRelease
                targetModifierKey.inBetweenPressRelease = true;
                nextState = ModifierLatchedState;
            }
        }
        setModifierState(targetModifierKey, nextState);
        filterNextKey = true;
    }
}

void DuiHardwareKeyboard::symbolKeyClick()
{
    qDebug() << __PRETTY_FUNCTION__;
    const int matchedIndex = redirectedKeyIndex(SymKey);
    if (!sensitiveKeys[matchedIndex].charKeyClicked) {
        emit symbolKeyClicked();
        // If there are some latched/locked modifiers
        // also need the next input key to be redirected to plugin.
        for (int i = 0; i < sensitiveKeys.count(); i++) {
            if ((sensitiveKeys[i].modifier.modifier != Qt::NoModifier)
                    && (sensitiveKeys[i].modifier.state != ModifierClearState)) {
                filterNextKey = true;
                break;
            }
        }
    } else {
        // Commits the accented character.
        commitAccentedCharacter();
    }
}

void DuiHardwareKeyboard::composeAccentedCharacter(Qt::Key /* keyCode */, const QString &text)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (text.isEmpty() && (text.length() != 1))
        return;

    // Sym + Character key to enter an accented character
    const int symKeyIndex = redirectedKeyIndex(SymKey);
    QChar character = text.at(0);
    const QString accentedCharacters = hwkbCharLoopsManager.characterLoop(character);
    const int accentedKeyCode = QKeySequence(character)[0];
    const bool sameKey = (sensitiveKeys[symKeyIndex].lastClickedCharacter == character);

    if (!accentedCharacters.isEmpty()) {
        if (sameKey) {
            // If there is only one accented character (already input), don't need update again.
            if (accentedCharacters.length() <= 1)
                return;
            const int index = sensitiveKeys[symKeyIndex].charKeyClickedCount % accentedCharacters.length();
            character = accentedCharacters[index];
        } else {
            character = accentedCharacters[0];
        }
    }

    if (!sameKey) {
        // Commits the accented character
        commitAccentedCharacter();
    }
    // Inserts the accented character
    emit symbolCharacterKeyClicked(character, accentedKeyCode, false);
}

void DuiHardwareKeyboard::characterKeyClick(Qt::Key keyCode, const QString &text)
{
    //clear all the modifiers in latched state, and handle symbol + character.
    for (int i = 0; i < sensitiveKeys.count(); i++) {
        if (sensitiveKeys[i].modifier.modifier != Qt::NoModifier) {
            if (sensitiveKeys[i].modifier.state == ModifierLatchedState) {
                //latched -> clear
                setModifierState(sensitiveKeys[i].modifier, ModifierClearState);
            }
        } else if (sensitiveKeys[i].pressed) {
            //symbol + character
            if (sensitiveKeys[i].keyCode == SymKey) {
                composeAccentedCharacter(keyCode, text);
            }
            //record there is a character key being pressed, to avoid to show/switch symbol view.
            sensitiveKeys[i].charKeyClicked = true;
            if (!text.isEmpty() && (text.at(0) == sensitiveKeys[i].lastClickedCharacter)) {
                ++sensitiveKeys[i].charKeyClickedCount;
            } else {
                //new character key
                if (text.isEmpty())
                    sensitiveKeys[i].lastClickedCharacter = QChar();
                else
                    sensitiveKeys[i].lastClickedCharacter = text.at(0);
                sensitiveKeys[i].charKeyClickedCount = 1;
            }
        }
    }
}

bool DuiHardwareKeyboard::symViewAvailable() const
{
    bool available = true;
    switch (keyboardType) {
    case Dui::NumberContentType:
    case Dui::PhoneNumberContentType:
        available = false;
        break;
    default:
        break;
    }
    return available;
}

bool DuiHardwareKeyboard::isValidModifierState(Qt::KeyboardModifier modifier, ModifierState state)
{
    bool valid = false;
    switch (keyboardType) {
    case Dui::NumberContentType:
        // number content type keeps fn lock, disable other modifier and other state
        if (modifier == FnLevelModifier && state == ModifierLockedState)
            valid = true;
        break;
    default:
        valid = true;
        break;
    }
    return valid;
}

void DuiHardwareKeyboard::handleIndicatorButtonClick()
{
    qDebug() << __PRETTY_FUNCTION__;
    //input mode indicator interactions
    //TODO: indicator button click in deadkey mode
    int index = -1;
    for (int i = 0; i < sensitiveKeys.count(); i++) {
        if (sensitiveKeys[i].modifier.modifier != Qt::NoModifier
                && sensitiveKeys[i].modifier.state != ModifierClearState) {
            index = i;
            break;
        }
    }
    if (index == -1)
        index = redirectedKeyIndex(Qt::ShiftModifier);
    ModifierKey &targetModifierKey = sensitiveKeys[index].modifier;

    modifierKeyPress(targetModifierKey);
    modifierKeyRelease(targetModifierKey);
}

void DuiHardwareKeyboard::commitAccentedCharacter()
{
    qDebug() << __PRETTY_FUNCTION__;
    const int symIndex = redirectedKeyIndex(SymKey);
    if (sensitiveKeys[symIndex].lastClickedCharacter.isNull())
        return;

    QChar character = sensitiveKeys[symIndex].lastClickedCharacter;
    const QString accentedCharacters =
        hwkbCharLoopsManager.characterLoop(sensitiveKeys[symIndex].lastClickedCharacter);
    if (!accentedCharacters.isEmpty()) {
        const int index = (sensitiveKeys[symIndex].charKeyClickedCount - 1) % accentedCharacters.length();
        if (index >= 0)
            character = accentedCharacters[index];
    }
    const int accentedKeyCode = QKeySequence(character)[0];
    emit symbolCharacterKeyClicked(character, accentedKeyCode, true);
}
