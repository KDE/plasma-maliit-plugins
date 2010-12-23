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

#ifndef POPUPBASE_H
#define POPUPBASE_H

class MImAbstractKey;
class MImAbstractKeyArea;

class QPointF;
class QPoint;
class QSize;
class QString;


//! \brief Base class for popup implementation
class PopupBase
{
public:
    //! Constructor
    //! \param mainArea Allows PopupBase to forward requests to the main area
    explicit PopupBase(const MImAbstractKeyArea  *mainArea);

    //! \brief Destructor
    virtual ~PopupBase();

    //! \brief Sets popup position at specified key in according to current orientation
    //! \param keyPos key's position
    //! \param screenPos key's position on the screen
    //! \param keySize  key's size
    virtual void updatePos(const QPointF &keyPos,
                           const QPoint &screenPos,
                           const QSize &keySize) = 0;

    //! \brief Cancel PopupBase actions
    virtual void cancel() = 0;

    //! \brief Allows PopupBase to act upon key-pressed on the main area
    //! \param keyPos key's position
    //! \param screenPos key's position on the screen
    //! \param keySize  key's size
    virtual void handleKeyPressedOnMainArea(MImAbstractKey *key,
                                            const QString &accent,
                                            bool upperCase) = 0;

    //! \brief Allows PopupBase to act upon long key-pressed on the main area
    //! \param keyPos key's position
    //! \param screenPos key's position on the screen
    //! \param keySize  key's size
    virtual void handleLongKeyPressedOnMainArea(MImAbstractKey *key,
                                                const QString &accent,
                                                bool upperCase) = 0;

    //! Returns whether PopupBase has any visible components
    virtual bool isVisible() const = 0;

    //! Toggles visibility of PopupBase
    virtual void setVisible(bool visible) = 0;
};

#endif

