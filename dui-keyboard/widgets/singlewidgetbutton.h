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



#ifndef SINGLEWIDGETBUTTON_H
#define SINGLEWIDGETBUTTON_H

#include "ikeybutton.h"

class DuiVirtualKeyboardStyleContainer;
class QGraphicsItem;

//! Represents a key model with the key's current binding state, and also contains its visible area.
class SingleWidgetButton : public IKeyButton
{
public:
    SingleWidgetButton(const VKBDataKey &key,
                       const DuiVirtualKeyboardStyleContainer &style,
                       QGraphicsItem &parent);
    virtual ~SingleWidgetButton();

    //! \reimp
    virtual const QString label() const;
    virtual const QString secondaryLabel() const;
    virtual QRect buttonRect() const;
    virtual QRect buttonBoundingRect() const;
    virtual void setModifiers(bool shift, QChar accent = QChar());
    virtual void setDownState(bool down);
    virtual void setSelected(bool select);
    virtual ButtonState state() const;
    virtual const VKBDataKey &key() const;
    virtual const KeyBinding &binding() const;
    virtual bool isDeadKey() const;
    //! \reimp_end

    //! \brief Returns the icon of this button, if it has one.
    const QPixmap *icon() const;

    //! \brief Draws the icon of this button, if it has one, to the given rectangle.
    void drawIcon(const QRect &rectangle, QPainter *painter) const;

    //! \brief Calls parent item's QGraphicsItem::update() who actually draws the button.
    void update();

    //! Cache for the buttons position and size. They can always
    //! be calculated but are faster to access this way.
    QRect cachedBoundingRect;
    QRect cachedButtonRect;

    //! The width for this button. Not managed by this class.
    //! It is used by SingleWidgetButtonArea to store the correct button size.
    int width;

private:
    const QPixmap *loadIcon(KeyBinding::KeyAction action, bool shift) const;

    //! The key this button represents
    const VKBDataKey &dataKey;

    bool shift;
    QChar accent;

    QString currentLabel;
    ButtonState currentState;
    bool selected;

    //! One icon for both shift states.
    const QPixmap *icons[2];

    const DuiVirtualKeyboardStyleContainer &styleContainer;

    QGraphicsItem &parentItem;
};

#endif // SINGLEWIDGETBUTTON_H

