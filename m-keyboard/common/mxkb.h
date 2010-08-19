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

#ifndef MXKB_H
#define MXKB_H

class QString;
/*!
  \brief MXkb provides some functionalities to manipulate the Xkb keyboard.

  Class MXkb provides some functionality to manipulate the Xkb keyboard,
  e.g. latch/unlatch, lock/unlock Modifier keys.
*/
class MXkb
{
public:
    //! \brief Constructor.
    MXkb();

    //! Destructor.
    ~MXkb();

    /*! \brief XkbLatchModifiers wrapper
     *
     * Set latch state of of modifiers indicated by \a affect mask to
     * what is indicated by \a values mask.
     */
    void latchModifiers(unsigned int affect, unsigned int values);
    //! \brief Just like \a latchModifiers but change the lock state.
    void lockModifiers(unsigned int affect, unsigned int values);

    /*!
     * \brief set the keyboard using the X Keyboard Extension.
     *
     * \param model the name of the keyboard model used to determine the components.
     * \param layout the name of the layout used todetermine the components
     *  which make up the keyboard description.
     * \param variant  Specifies which variant of the keyboard layout should
     *  be used to determine the components which make up the keyboard description.
     *
     * \return true if success.
     */
    bool setXkbMap(const QString &model, const QString &layout, const QString &variant);

private:
    unsigned int deviceSpec;

    friend class Ft_MXkb;
    friend class Ft_MHardwareKeyboard;
};

#endif
