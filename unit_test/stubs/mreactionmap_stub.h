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


#ifndef MREACTIONMAP_STUB_H
#define MREACTIONMAP_STUB_H

#include <mreactionmap.h>

/*!
 * \brief MReactionMap stub class.
 *
 * To fake MReactionMap operations, derive from this class
 * and implement the methods you want to fake. Instantiate your
 * derived stub class and assign it to gMReactionMapStub
 * global variable.
 */
class MReactionMapStub
{
public:
    MReactionMapStub();
    virtual ~MReactionMapStub();

    virtual void mreactionMapConstructor(QWidget *topLevelWidget, const QString &appIdentifier, QObject *parent);
    virtual void mreactionMapDestructor();

    virtual MReactionMap *instance(QWidget *anyWidget);
    virtual void setInactiveDrawingValue();
    virtual void setReactiveDrawingValue();
    virtual void setTransparentDrawingValue();
    virtual void setDrawingValue(const QString &pressFeedback, const QString &releaseFeedback);
    virtual QTransform transform() const;
    virtual void setTransform(QTransform transform);
    virtual void setTransform(const QGraphicsItem *item, const QGraphicsView *view);
    virtual void fillRectangle(int x, int y, int width, int height);
    virtual void fillRectangle(const QRect &rectangle);
    virtual void fillRectangle(const QRectF &rectangle);
    virtual void fillRectangle(const QRect &rectangle, const QString &pressFeedback, const QString &releaseFeedback);
    virtual void fillRectangle(const QRectF &rectangle, const QString &pressFeedback, const QString &releaseFeedback);
    virtual int width() const;
    virtual int height() const;
    virtual void clear();
};

MReactionMapStub::MReactionMapStub()
{
}

MReactionMapStub::~MReactionMapStub()
{
}

void MReactionMapStub::mreactionMapConstructor(QWidget */*topLevelWidget*/, const QString &/*appIdentifier*/, QObject */*parent*/)
{
}

void MReactionMapStub::mreactionMapDestructor()
{
}

MReactionMap *MReactionMapStub::instance(QWidget */*anyWidget*/)
{
    return 0;
}

void MReactionMapStub::setInactiveDrawingValue()
{
}

void MReactionMapStub::setReactiveDrawingValue()
{
}

void MReactionMapStub::setTransparentDrawingValue()
{
}

void MReactionMapStub::setDrawingValue(const QString &/*pressFeedback*/, const QString &/*releaseFeedback*/)
{
}

QTransform MReactionMapStub::transform() const
{
    return QTransform();
}

void MReactionMapStub::setTransform(QTransform /*transform*/)
{
}

void MReactionMapStub::setTransform(const QGraphicsItem */*item*/, const QGraphicsView */*view*/)
{
}

void MReactionMapStub::fillRectangle(int /*x*/, int /*y*/, int /*width*/, int /*height*/)
{
}

void MReactionMapStub::fillRectangle(const QRect &/*rectangle*/)
{
}

void MReactionMapStub::fillRectangle(const QRectF &/*rectangle*/)
{
}

void MReactionMapStub::fillRectangle(const QRect &/*rectangle*/, const QString &/*pressFeedback*/, const QString &/*releaseFeedback*/)
{
}

void MReactionMapStub::fillRectangle(const QRectF &/*rectangle*/, const QString &/*pressFeedback*/, const QString &/*releaseFeedback*/)
{
}

int MReactionMapStub::width() const
{
    return 0;
}

int MReactionMapStub::height() const
{
    return 0;
}

void MReactionMapStub::clear()
{
}

MReactionMapStub gDefaultMReactionMapStub;

/*
 * This is the stub class instance used by the system. If you want to alter behaviour,
 * derive your stub class from MReactionMapStub, implement the methods you want to
 * fake, create an instance of your stub class and assign the instance into this global variable.
 */
MReactionMapStub *gMReactionMapStub = &gDefaultMReactionMapStub;

/*
 * These are the proxy method implementations of MReactionMap. They will
 * call the stub object methods of the gMReactionMapStub.
 */

MReactionMap::MReactionMap(QWidget *topLevelWidget, const QString &appIdentifier, QObject *parent)
{
    gMReactionMapStub->mreactionMapConstructor(topLevelWidget, appIdentifier, parent);
}

MReactionMap::~MReactionMap()
{
    gMReactionMapStub->mreactionMapDestructor();
}

MReactionMap *MReactionMap::instance(QWidget *anyWidget)
{
    return gMReactionMapStub->instance(anyWidget);
}

void MReactionMap::setInactiveDrawingValue()
{
    gMReactionMapStub->setInactiveDrawingValue();
}

void MReactionMap::setReactiveDrawingValue()
{
    gMReactionMapStub->setReactiveDrawingValue();
}

void MReactionMap::setTransparentDrawingValue()
{
    gMReactionMapStub->setTransparentDrawingValue();
}

void MReactionMap::setDrawingValue(const QString &pressFeedback, const QString &releaseFeedback)
{
    gMReactionMapStub->setDrawingValue(pressFeedback, releaseFeedback);
}

QTransform MReactionMap::transform() const
{
    return gMReactionMapStub->transform();
}

void MReactionMap::setTransform(QTransform transform)
{
    gMReactionMapStub->setTransform(transform);
}

void MReactionMap::setTransform(QGraphicsItem *item, QGraphicsView *view)
{
    gMReactionMapStub->setTransform(item, view);
}

void MReactionMap::fillRectangle(int x, int y, int width, int height)
{
    gMReactionMapStub->fillRectangle(x, y, width, height);
}

void MReactionMap::fillRectangle(const QRect &rectangle)
{
    gMReactionMapStub->fillRectangle(rectangle);
}

void MReactionMap::fillRectangle(const QRectF &rectangle)
{
    gMReactionMapStub->fillRectangle(rectangle);
}

void MReactionMap::fillRectangle(const QRect &rectangle, const QString &pressFeedback, const QString &releaseFeedback)
{
    gMReactionMapStub->fillRectangle(rectangle, pressFeedback, releaseFeedback);
}

void MReactionMap::fillRectangle(const QRectF &rectangle, const QString &pressFeedback, const QString &releaseFeedback)
{
    gMReactionMapStub->fillRectangle(rectangle, pressFeedback, releaseFeedback);
}

int MReactionMap::width() const
{
    return gMReactionMapStub->width();
}

int MReactionMap::height() const
{
    return gMReactionMapStub->height();
}

void MReactionMap::clear()
{
    gMReactionMapStub->clear();
}

#endif // MREACTIONMAP_STUB_H
