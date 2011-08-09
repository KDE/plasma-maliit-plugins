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

#include "mimsnapshotpixmapitem.h"
#include "mplainwindow.h"
#include "panparameters.h"

#include <MNamespace>
#include <QDebug>
#include <MScene>
#include <MSceneManager>

namespace
{
}

MImSnapshotPixmapItem::MImSnapshotPixmapItem(const QPixmap &pixmap,
                                             QGraphicsItem* parent)
    : QObject(),
      QGraphicsPixmapItem(pixmap, parent)
{
}

MImSnapshotPixmapItem::MImSnapshotPixmapItem(QGraphicsItem *parent)
    : QObject(),
      QGraphicsPixmapItem(parent)
{
}

MImSnapshotPixmapItem::~MImSnapshotPixmapItem()
{
    connectPanParameters(0);
}

void MImSnapshotPixmapItem::grabScreen(const QRect &rect)
{
    QPixmap pixmap = QPixmap::grabWidget(scene()->views().at(0), rect);

    if (MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait) {
        QTransform portraitTransform;
        portraitTransform.rotate(90);
        pixmap = pixmap.transformed(portraitTransform);
    }

    setPixmap(pixmap);
}

void MImSnapshotPixmapItem::grabWidgets(QList<QPointer<QGraphicsWidget> > widgets)
{
    if (!widgets.count()) {
        // clear by setting empty pixmap
        setPixmap(QPixmap());
        return;
    }

    bool isPortrait =
        (MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait);
    const QRectF screenRect
        = QRectF(QPoint(0, 0),
                 MPlainWindow::instance()->visibleSceneSize(M::Landscape));

    QRectF rect;
    foreach (const QGraphicsWidget *widget, widgets) {
        rect |= (widget->sceneBoundingRect() & screenRect);
    }

    qreal posY = 0;
    if (isPortrait) {
        posY = MPlainWindow::instance()->visibleSceneSize(M::Landscape).width()
                - rect.width();
        rect =QRectF(0, 0, rect.height(), rect.width());
    } else {
        posY = MPlainWindow::instance()->visibleSceneSize(M::Landscape).height()
                - rect.height();
    }
    QPoint pos(0, posY);

    QPixmap pixmap(rect.width(), rect.height());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    foreach (QGraphicsWidget *widget, widgets) {
        QPixmap tp(widget->size().toSize());
        QPainter p(&tp);
        // The children must be inside parent widget, otherwise it only
        // grab children content in side parent widget to the pixmap.
        foreach (QGraphicsItem *item, widget->childItems())
            item->paint(&p, 0, 0);
        widget->paint(&p, 0, 0);

        if (isPortrait) {
            painter.drawPixmap(QPointF(0, widget->scenePos().x() - pos.y()),
                               tp);
        } else {
            painter.drawPixmap(QPointF(widget->scenePos() - pos), tp);
        }
    }

    setPixmap(pixmap);
}

void MImSnapshotPixmapItem::grabPixmaps(QMap<QPixmap*, QPoint > pixmaps)
{
    qDebug() << __PRETTY_FUNCTION__ << " with pixmaps:" <<  pixmaps.count();
    if (!pixmaps.count()) {
        // clear by setting empty pixmap
        setPixmap(QPixmap());
        return;
    }

    bool isPortrait =
        (MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait);

    QRect rect;
    QMap<QPixmap*, QPoint>::const_iterator it = pixmaps.constBegin();
    QPoint basePos = it.value();

    for (it = pixmaps.constBegin(); it != pixmaps.constEnd(); it++) {
        if (!it.key())
            continue;
        rect |= QRect(it.value(), it.key()->size());
        basePos.setX(qMin(it.value().x(), basePos.x()));
        basePos.setY(qMin(it.value().y(), basePos.y()));
    }

    QSize size = isPortrait ? QSize(rect.height(), rect.width()) : rect.size();
    QPixmap pixmap(size);
    QPainter painter(&pixmap);

    if (isPortrait) {
        QTransform portraitTransform;
        portraitTransform.rotate(90);
        for (it = pixmaps.constBegin(); it != pixmaps.constEnd(); it++) {
            if (!it.key())
                continue;
            painter.drawPixmap(QPointF(it.value() - basePos),
                               QPixmap(*it.key()).transformed(portraitTransform));
        }
    } else {
        for (it = pixmaps.constBegin(); it != pixmaps.constEnd(); it++) {
            if (!it.key())
                continue;
            painter.drawPixmap(QPointF(it.value() - basePos),
                               *it.key());
        }
    }

    setPixmap(pixmap);
}

void MImSnapshotPixmapItem::updatePos(const QPointF &pos)
{
    setPos(pos);
}

void MImSnapshotPixmapItem::updateOpacity(qreal opacity)
{
    setOpacity(opacity);
}

void MImSnapshotPixmapItem::updateScale(qreal scale)
{
    setScale(scale);
}

void MImSnapshotPixmapItem::connectPanParameters(PanParameters *parameters)
{
    if (!mParameters.isNull()) {
        disconnect(mParameters.data(), 0, this, 0);
    }
    mParameters = parameters;

    if (!mParameters.isNull())  {
        connect(parameters, SIGNAL(positionChanged(QPointF)),
                this,       SLOT(updatePos(QPointF)));

        connect(parameters, SIGNAL(opacityChanged(qreal)),
                this,       SLOT(updateOpacity(qreal)));

        connect(parameters, SIGNAL(scaleChanged(qreal)),
                this,       SLOT(updateScale(qreal)));
    }
}
