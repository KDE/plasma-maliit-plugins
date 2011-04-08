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


#ifndef MREACTIONMAP_STUB_H
#define MREACTIONMAP_STUB_H

#include <QtCore>
#include <QtGui>

#ifdef HAVE_REACTIONMAP
#include <mreactionmap.h>
#else
class MReactionMapPrivate
{};

class MReactionMap
    : public QObject
{
    Q_OBJECT

private:
    MReactionMapPrivate *d;

public:
    static const QString Press;
    static const QString Release;
    static const QString Cancel;
    static const QString Transparent;
    static const QString Inactive;

    explicit MReactionMap(QWidget *topLevelWidget,
                          const QString &appIdentifier = QString(),
                          QObject *parent = 0);

    ~MReactionMap();

    static MReactionMap *instance(QWidget *anyWidget);
    void setInactiveDrawingValue();
    void setReactiveDrawingValue();
    void setTransparentDrawingValue();
    void setDrawingValue(const QString &pressFeedback,
                         const QString &releaseFeedback);
    QTransform transform() const;
    void setTransform(QTransform transform);
    void setTransform(QGraphicsItem *item,
                      QGraphicsView *view);
    void fillRectangle(int x,
                       int y,
                       int width,
                       int height);
    void fillRectangle(const QRect &rectangle);
    void fillRectangle(const QRectF &rectangle);
    void fillRectangle(const QRect &rectangle,
                       const QString &pressFeedback,
                       const QString &releaseFeedback);
    void fillRectangle(const QRectF &rectangle,
                       const QString &pressFeedback,
                       const QString &releaseFeedback);
    int width() const;
    int height() const;
    void clear();
};

const QString MReactionMap::Press = "press";
const QString MReactionMap::Release = "release";
const QString MReactionMap::Cancel = "cancel";
const QString MReactionMap::Transparent = "transparent";
const QString MReactionMap::Inactive = "";
#endif

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
    : d(0)
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
