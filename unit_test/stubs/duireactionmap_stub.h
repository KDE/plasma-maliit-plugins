/* * This file is part of m-keyboard *
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


#ifndef DUIREACTIONMAP_STUB_H
#define DUIREACTIONMAP_STUB_H

#include <duireactionmap.h>

/*!
 * \brief DuiReactionMap stub class.
 *
 * To fake DuiReactionMap operations, derive from this class
 * and implement the methods you want to fake. Instantiate your
 * derived stub class and assign it to gDuiReactionMapStub
 * global variable.
 */
class DuiReactionMapStub
{
public:
    DuiReactionMapStub();
    virtual ~DuiReactionMapStub();

    virtual void duireactionMapConstructor(QWidget *topLevelWidget, const QString &appIdentifier, QObject *parent);
    virtual void duireactionMapDestructor();

    virtual DuiReactionMap *instance(QWidget *anyWidget);
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

DuiReactionMapStub::DuiReactionMapStub()
{
}

DuiReactionMapStub::~DuiReactionMapStub()
{
}

void DuiReactionMapStub::duireactionMapConstructor(QWidget */*topLevelWidget*/, const QString &/*appIdentifier*/, QObject */*parent*/)
{
}

void DuiReactionMapStub::duireactionMapDestructor()
{
}

DuiReactionMap *DuiReactionMapStub::instance(QWidget */*anyWidget*/)
{
    return 0;
}

void DuiReactionMapStub::setInactiveDrawingValue()
{
}

void DuiReactionMapStub::setReactiveDrawingValue()
{
}

void DuiReactionMapStub::setTransparentDrawingValue()
{
}

void DuiReactionMapStub::setDrawingValue(const QString &/*pressFeedback*/, const QString &/*releaseFeedback*/)
{
}

QTransform DuiReactionMapStub::transform() const
{
    return QTransform();
}

void DuiReactionMapStub::setTransform(QTransform /*transform*/)
{
}

void DuiReactionMapStub::setTransform(const QGraphicsItem */*item*/, const QGraphicsView */*view*/)
{
}

void DuiReactionMapStub::fillRectangle(int /*x*/, int /*y*/, int /*width*/, int /*height*/)
{
}

void DuiReactionMapStub::fillRectangle(const QRect &/*rectangle*/)
{
}

void DuiReactionMapStub::fillRectangle(const QRectF &/*rectangle*/)
{
}

void DuiReactionMapStub::fillRectangle(const QRect &/*rectangle*/, const QString &/*pressFeedback*/, const QString &/*releaseFeedback*/)
{
}

void DuiReactionMapStub::fillRectangle(const QRectF &/*rectangle*/, const QString &/*pressFeedback*/, const QString &/*releaseFeedback*/)
{
}

int DuiReactionMapStub::width() const
{
    return 0;
}

int DuiReactionMapStub::height() const
{
    return 0;
}

void DuiReactionMapStub::clear()
{
}

DuiReactionMapStub gDefaultDuiReactionMapStub;

/*
 * This is the stub class instance used by the system. If you want to alter behaviour,
 * derive your stub class from DuiReactionMapStub, implement the methods you want to
 * fake, create an instance of your stub class and assign the instance into this global variable.
 */
DuiReactionMapStub *gDuiReactionMapStub = &gDefaultDuiReactionMapStub;

/*
 * These are the proxy method implementations of DuiReactionMap. They will
 * call the stub object methods of the gDuiReactionMapStub.
 */

DuiReactionMap::DuiReactionMap(QWidget *topLevelWidget, const QString &appIdentifier, QObject *parent)
{
    gDuiReactionMapStub->duireactionMapConstructor(topLevelWidget, appIdentifier, parent);
}

DuiReactionMap::~DuiReactionMap()
{
    gDuiReactionMapStub->duireactionMapDestructor();
}

DuiReactionMap *DuiReactionMap::instance(QWidget *anyWidget)
{
    return gDuiReactionMapStub->instance(anyWidget);
}

void DuiReactionMap::setInactiveDrawingValue()
{
    gDuiReactionMapStub->setInactiveDrawingValue();
}

void DuiReactionMap::setReactiveDrawingValue()
{
    gDuiReactionMapStub->setReactiveDrawingValue();
}

void DuiReactionMap::setTransparentDrawingValue()
{
    gDuiReactionMapStub->setTransparentDrawingValue();
}

void DuiReactionMap::setDrawingValue(const QString &pressFeedback, const QString &releaseFeedback)
{
    gDuiReactionMapStub->setDrawingValue(pressFeedback, releaseFeedback);
}

QTransform DuiReactionMap::transform() const
{
    return gDuiReactionMapStub->transform();
}

void DuiReactionMap::setTransform(QTransform transform)
{
    gDuiReactionMapStub->setTransform(transform);
}

void DuiReactionMap::setTransform(QGraphicsItem *item, QGraphicsView *view)
{
    gDuiReactionMapStub->setTransform(item, view);
}

void DuiReactionMap::fillRectangle(int x, int y, int width, int height)
{
    gDuiReactionMapStub->fillRectangle(x, y, width, height);
}

void DuiReactionMap::fillRectangle(const QRect &rectangle)
{
    gDuiReactionMapStub->fillRectangle(rectangle);
}

void DuiReactionMap::fillRectangle(const QRectF &rectangle)
{
    gDuiReactionMapStub->fillRectangle(rectangle);
}

void DuiReactionMap::fillRectangle(const QRect &rectangle, const QString &pressFeedback, const QString &releaseFeedback)
{
    gDuiReactionMapStub->fillRectangle(rectangle, pressFeedback, releaseFeedback);
}

void DuiReactionMap::fillRectangle(const QRectF &rectangle, const QString &pressFeedback, const QString &releaseFeedback)
{
    gDuiReactionMapStub->fillRectangle(rectangle, pressFeedback, releaseFeedback);
}

int DuiReactionMap::width() const
{
    return gDuiReactionMapStub->width();
}

int DuiReactionMap::height() const
{
    return gDuiReactionMapStub->height();
}

void DuiReactionMap::clear()
{
    gDuiReactionMapStub->clear();
}

#endif // DUIREACTIONMAP_STUB_H
