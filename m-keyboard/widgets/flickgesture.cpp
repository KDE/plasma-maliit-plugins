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
    : QGesture(parent)
{
}

FlickGesture::~FlickGesture()
{
}

QPointF FlickGesture::positionDifference() const
{
    return endStartDifference;
}

void FlickGesture::setPositionDifference(const QPointF &positionDifference)
{
    endStartDifference = positionDifference;
}

FlickGesture::Direction FlickGesture::direction() const
{
    return gestureDirection;
}

void FlickGesture::setDirection(const Direction newDirection)
{
    gestureDirection = newDirection;
}

