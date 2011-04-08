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

    /*! \brief XkbLockModifiers wrapper
     *
     * Set latch state of of modifiers indicated by \a affect mask to
     * what is indicated by \a values mask.
     */
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
