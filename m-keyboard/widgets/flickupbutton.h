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



#ifndef FLICKUPBUTTON_H
#define FLICKUPBUTTON_H

#include <MButton>
#include "ikeybutton.h"

class VKBDataKey;
class KeyBinding;

/*!
 * \class FlickUpButton
 * \brief FlickUpButton is vkb push button with icon view
 *
 *  Such buttons emit signal flicked() when mouse is released
 *  above the button
 */
class FlickUpButton : public MButton, public IKeyButton
{
    Q_OBJECT
public:
    /*!
     * \Brief Constructor
     * \param name QString object name
     * \param parent MWidget* parent widget
     */
    explicit FlickUpButton(const VKBDataKey &dataKey, QGraphicsItem *parent = 0);

    //! Destructor
    virtual ~FlickUpButton();

    //! Emits signal flicked()
    void flick();

    //! Returns true if flick is enabled
    bool getFlickEnabled() const;

    //! Disables flick
    void disableFlick();

    //! Enables flick
    void enableFlick();

    /*!
     * \brief Method to handle mouse movement
     * \param pos QPointF current pointer position
     */
    bool onMove(QPointF pos);

    //! \reimp
    virtual const QString label() const;
    virtual const QString secondaryLabel() const;
    virtual QRectF buttonRect() const;
    virtual QRectF buttonBoundingRect() const;
    virtual void setModifiers(bool shift, QChar accent = QChar());
    virtual void setSelected(bool select);
    virtual void setDownState(bool down);
    virtual ButtonState state() const;
    virtual const VKBDataKey &key() const;
    virtual const KeyBinding &binding() const;
    virtual bool isDeadKey() const;
    //! \reimp_end

signals:
    //! Signal is emitted when mouse is released above the button
    void flicked();

private:
    void updateButtonText();
    void updateButtonObjectName();

private:
    //! Contains TRUE if flick is enabled
    bool flickEnabled;

    //! The key this button represents
    const VKBDataKey &dataKey;

    bool shift;
    QChar accent;
};

#endif

