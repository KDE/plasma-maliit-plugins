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
