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
#include <QPoint>

/*!
  \brief Meego keyboard flick gesture.

  Flick gestures are simple swipe gestures, made with just one finger.
*/
class FlickGesture : public QGesture
{
public:
    //! Possible gesture directions
    enum Direction {
        Left,
        Right,
        Up,
        Down,
        NoDirection,
    };

    Q_OBJECT
    Q_DISABLE_COPY(FlickGesture)

public:
    //! Constructor
    FlickGesture(QObject *parent = 0);

    //! Destructor
    virtual ~FlickGesture();

    Direction direction() const;

    //! Distance traveled in pixels to major direction
    int distance() const;

    QPoint startPosition() const;

    QPoint currentPosition() const;

    int elapsedTime() const;

private:
    // These members are set directly by the recognizer
    int startTime; //! Start time in milliseconds
    int currentTime; //! End/current time in milliseconds
    QPoint startPos;
    QPoint currentPos;
    Direction dir;
    Direction prevDir;
    int dist;
    int prevDist;
    bool hasZigZagged;
    bool isAccidentallyFlicked;
    bool pressReceived;

    friend class FlickGestureRecognizer;
    friend class Ut_MImAbstractKeyArea;
};

#endif
