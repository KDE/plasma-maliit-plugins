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

#ifndef MIMABSTRACTPOPUP_H
#define MIMABSTRACTPOPUP_H

class KeyContext;
class MImAbstractKey;
class MImAbstractKeyArea;

class QPointF;
class QPoint;
class QSize;
class QString;


//! \brief Base class for popup implementation
class MImAbstractPopup
{
public:
    virtual ~MImAbstractPopup() = 0;

    enum EffectOnKey {
        NoEffect,        //! No effect on pressed key state.
        ResetPressedKey, //! Reset pressed key state.
    };


    //! \brief Set main area
    //!
    //! \param mainArea Allows MImAbstractPopup to forward requests to the
    //! main area. Must assign popup instance ownership to mainArea.
    virtual void setMainArea(MImAbstractKeyArea *mainArea) = 0;

    //! \brief Sets popup position at specified key in according to current orientation
    //! \param keyPos key's position
    //! \param screenPos key's position on the screen
    //! \param keySize  key's size
    virtual void updatePos(const QPointF &keyPos,
                           const QPoint &screenPos,
                           const QSize &keySize) = 0;

    //! \brief Cancel MImAbstractPopup actions
    virtual void cancel() = 0;

    //! \brief Allows MImAbstractPopup to act upon key-pressed on the main area
    //! \param keyPos key's position
    //! \param keyContext Context information at the moment key was pressed
    virtual void handleKeyPressedOnMainArea(MImAbstractKey *key,
                                            const KeyContext &keyContext) = 0;

    //! \brief Allows MImAbstractPopup to act upon long key-pressed on the main area
    //! \param keyPos key's position
    //! \param keyContext Context information at the moment key was long-pressed
    virtual EffectOnKey handleLongKeyPressedOnMainArea(MImAbstractKey *key,
                                                       const KeyContext &keyContext) = 0;

    //! Returns whether MImAbstractPopup has any visible components
    virtual bool isVisible() const = 0;

    //! Toggles visibility of MImAbstractPopup
    virtual void setVisible(bool visible) = 0;
};

inline MImAbstractPopup::~MImAbstractPopup()
{}

#endif // MIMABSTRACTPOPUP_H
