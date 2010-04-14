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

#ifndef MXKB_H
#define MXKB_H

#include <QKeyEvent>

/*!
  \brief MXkb provides some functionalities to manipulate the Xkb keyboard.

  Class MXkb provides some functionalities to manipulate the Xkb keyboard, e.g. latch/unlatch, lock/unlock
  Modifier keys.
*/
class MXkbPrivate;
class MXkb
{
public:
    /*!
     * \brief Constructor.
     */
    MXkb();

    //! Destructor
    ~MXkb();

    /*!
     * \brief lock \a modifiers.
     */
    void lockModifiers(Qt::KeyboardModifiers modifiers);

    /*!
     * \brief unlock \a modifiers.
     */
    void unlockModifiers(Qt::KeyboardModifiers modifiers);

private:

    //! For unit test, returns true if \a modifier is in latched or locked state.
    bool isLatched(Qt::KeyboardModifier modifier) const;

    MXkbPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(MXkb)

    friend class Ft_MXkb;
    friend class Ft_MHardwareKeyboard;
};

#endif
