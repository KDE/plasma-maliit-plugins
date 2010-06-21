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

#ifndef HANDLE_H
#define HANDLE_H

#include "handlestyle.h"
#include <MStylableWidget>

class QGraphicsLinearLayout;
class FlickGesture;

/*!
  \brief Handle is a wrapper widget of zero or one child that reacts to flick gestures.

  Handle doesn't have any non-default content except the child if one is set.
*/
class Handle : public MStylableWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     * \param parent Parent object.
     */
    explicit Handle(QGraphicsWidget *parent = 0);

    //! Destructor
    virtual ~Handle();

    //! Set the one child object that Handle can contain.
    void setChild(QGraphicsLayoutItem *widget);

    //! \reimp
    bool event(QEvent *e);
    //! \reimp_end

signals:
    void flickUp(const FlickGesture &gesture);
    void flickDown(const FlickGesture &gesture);
    void flickLeft(const FlickGesture &gesture);
    void flickRight(const FlickGesture &gesture);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

protected:
    M_STYLABLE_WIDGET(HandleStyle)

private:
    void flickGestureEvent(FlickGesture &gesture);

private:
    QGraphicsLinearLayout &mainLayout;
};

#endif
