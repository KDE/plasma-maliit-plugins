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

#ifndef FLICKGESTURE_H
#define FLICKGESTURE_H

#include <QGesture>
#include <QPointF>

/*!
  \brief Meego keyboard flick gesture.

  Flick gestures are simple swipe gestures, made with just one finger.
*/
class FlickGesture : public QGesture
{
    Q_OBJECT

public:
    //! Possible gesture directions
    enum Direction {
        Left,
        Right,
        Up,
        Down
    };

    //! Constructor
    FlickGesture(QObject *parent = 0);

    //! Destructor
    virtual ~FlickGesture();

    //! \return difference between end and start points
    QPointF positionDifference() const;

    //! Set difference between end and start points
    void setPositionDifference(const QPointF &positionDifference);

    //! \return gesture direction as determined from the position difference by the
    //! recognizer.  Detailed x & y movement can be inspected via positionDifference().
    Direction direction() const;

    //! Set gesture direction
    void setDirection(Direction newDirection);

private:
    //! Difference between end and start points
    QPointF endStartDifference;

    Direction gestureDirection;
};

#endif
