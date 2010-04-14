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



#ifndef POPUPBASE_H
#define POPUPBASE_H

#include <QPointF>
#include <QPoint>
#include <QSize>
#include <QString>
#include <QRectF>
#include <QPixmap>

class IKeyButton;
class MVirtualKeyboardStyleContainer;
class QGraphicsItem;

/*!
 * \brief Base class for popup implementation
 */
class PopupBase
{
public:
    //! Constructor
    PopupBase(const MVirtualKeyboardStyleContainer &styleContainer);

    //! \brief Destructor
    virtual ~PopupBase();

    //! \brief Sets the accurate position of finger relative to center of popup text
    virtual void setFingerPos(const QPointF &pos) = 0;

    //! \brief Sets popup content
    virtual void setTargetButton(const IKeyButton *key);

    /*!
     * \brief Sets popup position at specified key in according to current orientation
     * \param keyPos key's position
     * \param screenPos key's position on the screen
     * \param keySize  key's size
     */
    virtual void updatePos(QPointF keyPos, QPoint screenPos, const QSize &keySize) = 0;

    //! Hide widget
    virtual void hidePopup() = 0;

    //! Show widget
    virtual void showPopup() = 0;

    // Return true if widget is visible
    virtual bool isPopupVisible() const = 0;

protected:
    const MVirtualKeyboardStyleContainer &styleContainer;

#ifdef UNIT_TEST
    friend class Ut_IPopup;
#endif
};

#endif

