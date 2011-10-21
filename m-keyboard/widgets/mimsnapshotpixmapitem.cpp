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

void MImSnapshotPixmapItem::grabWidgets(const QList<QPointer<QGraphicsWidget> > &widgets)
{
    if (not MPlainWindow::instance() || widgets.isEmpty()) {
        // clear by setting empty pixmap
        setPixmap(QPixmap());
        return;
    }

    // Querying visible scene size for landscape orientation, because
    // widget->sceneBoundingRect() always returns the bounding rectangle of
    // the widget for landscape orientation:
    const QSize &visibleSceneSize(MPlainWindow::instance()->visibleSceneSize(M::Landscape));

    const bool isPortrait((MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait));
    const QRectF &screenRect(QRectF(QPoint(0, 0), visibleSceneSize));

    QRectF rect;
    foreach (const QGraphicsWidget *widget, widgets) {
        rect |= (widget->sceneBoundingRect() & screenRect);
    }

    qreal posY = 0;
    if (isPortrait) {
        posY = visibleSceneSize.width() - rect.width();
        rect = QRectF(0, 0, rect.height(), rect.width());
    } else {
        posY = visibleSceneSize.height() - rect.height();
    }

    const QPointF pos(0, posY);
    QPixmap pixmap(rect.width(), rect.height());

    if (pixmap.isNull()) {
        // Avoid painting on invalid pixmap => could crash otherwise.
        return;
    }

    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    foreach (QGraphicsWidget *widget, widgets) {
        const QPointF tp(isPortrait ? QPointF(0, widget->scenePos().x() - pos.y())
                                    : widget->scenePos() - pos);

        QTransform t;
        t.translate(tp.x(), tp.y());
        painter.setTransform(t);

        widget->paint(&painter, 0, 0);
        foreach (QGraphicsItem *item, widget->childItems()) {
            // If isPortrait is true, then keep in mind that "rect" is also
            // "rotated" to portrait mode and moved to the area covered by the
            // VKB  (on the display). We still need to adjust the item (in
            // scene position) to that area.
            const QPointF &itemPos(isPortrait ? QPointF(rect.width() - item->scenePos().y(),
                                                        item->scenePos().x() - pos.y())
                                              : item->scenePos() - pos);
            QTransform itemTransform;
            itemTransform.translate(itemPos.x(), itemPos.y());
            painter.setTransform(itemTransform);
            item->paint(&painter, 0, 0);
        }
    }

    setPixmap(pixmap);
}

void MImSnapshotPixmapItem::grabPixmaps(const QMap<QPixmap*, QPoint > &pixmaps)
{
    qDebug() << __PRETTY_FUNCTION__ << " with pixmaps:" <<  pixmaps.count();
    if (pixmaps.isEmpty()) {
        // clear by setting empty pixmap
        setPixmap(QPixmap());
        return;
    }

    const bool isPortrait =
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

    QPixmap pixmap(isPortrait ? QSize(rect.height(), rect.width())
                              : rect.size());

    if (pixmap.isNull()) {
        // Avoid painting on invalid pixmap => could crash otherwise.
        return;
    }

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
