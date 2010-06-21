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

#include "flickgesture.h"

FlickGesture::FlickGesture(QObject *parent)
    : QGesture(parent),
      startTime(0),
      currentTime(0),
      dir(NoDirection),
      prevDir(NoDirection),
      dist(0),
      prevDist(0),
      hasZigZagged(false),
      isAccidentallyFlicked(false),
      pressReceived(false)
{
}

FlickGesture::~FlickGesture()
{
}

FlickGesture::Direction FlickGesture::direction() const
{
    return dir;
}

int FlickGesture::distance() const
{
    return dist;
}

QPoint FlickGesture::startPosition() const
{
    return startPos;
}

QPoint FlickGesture::currentPosition() const
{
    return currentPos;
}

int FlickGesture::elapsedTime() const
{
    return (currentTime - startTime);
}

