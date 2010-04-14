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

#ifndef DUIXKB_P_H
#define DUIXKB_P_H

#include <QKeyEvent>
#include <X11/Xlib.h>

class DuiXkb;

class DuiXkbPrivate
{
public:
    /*!
     * \brief Constructor.
     */
    DuiXkbPrivate();

    //! Destructor
    ~DuiXkbPrivate();

    //! Translates the Qt::KeyboardModifiers to xkb modifier.
    unsigned int translateModifiers(Qt::KeyboardModifiers modifier) const;

private:
    //! Initialize
    void init();

    //! Obtains xmodifier flags
    void getMask();

    //! Sets xmodifier flags according \a sym and \a mask
    void setMask(const KeySym &sym, unsigned int mask);

    //! Tests whether xmodifier \a mod is latched.
    bool testModifierLatchedState(int xModifier) const;

    Display *display;
    unsigned int deviceSpec;
    unsigned char qtAltMask;
    unsigned char qtMetaMask;
    unsigned char qtModeSwitchMask;

    friend class DuiXkb;
    friend class Ft_DuiXkb;
    friend class Ft_DuiHardwareKeyboard;
};

#endif
