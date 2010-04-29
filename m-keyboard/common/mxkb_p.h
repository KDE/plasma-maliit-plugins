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

#ifndef MXKB_P_H
#define MXKB_P_H

#include <Qt>
#include <X11/Xlib.h>

class MXkb;

class MXkbPrivate
{
public:
    /*!
     * \brief Constructor.
     */
    MXkbPrivate();

    //! Destructor
    ~MXkbPrivate();

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

    friend class MXkb;
    friend class Ft_MXkb;
    friend class Ft_MHardwareKeyboard;
};

#endif
