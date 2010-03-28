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



#ifndef IKEYBUTTON_H
#define IKEYBUTTON_H

#include "vkbdatakey.h"

//! Represents a key model with the key's current binding state, and also contains its visible area.
class IKeyButton
{
public:
    enum ButtonState {
        Normal,
        Pressed,
        Selected
    };

    //! \brief Returns current label. It is affected by active modifiers.
    virtual const QString label() const = 0;

    //! \brief Returns the secondary label.
    virtual const QString secondaryLabel() const = 0;

    //! \brief Returns the smallest rectangle that contains the button.
    virtual QRect buttonRect() const = 0;

    //! \brief Returns the bounding rectangle for the button.
    virtual QRect buttonBoundingRect() const = 0;

    //! \brief Sets shift and accent. Affects label and/or icon.
    virtual void setModifiers(bool shift, QChar accent = QChar()) = 0;

    //! \brief Set the pressed-state of the button.
    virtual void setState(ButtonState state) = 0;

    //! \brief Returns the pressed-state of the button.
    virtual ButtonState state() const = 0;

    //! \return the key this button represents
    virtual const VKBDataKey &key() const = 0;

    //! \brief Get current active key binding.
    virtual const KeyBinding &binding() const = 0;

    //! \brief Tells whether this key represents a dead key.
    virtual bool isDeadKey() const = 0;
};

#endif
