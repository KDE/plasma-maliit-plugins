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

#ifndef MIMSNAPSHOTPIXMAPITEM_H
#define MIMSNAPSHOTPIXMAPITEM_H

#include <QPixmap>
#include <QObject>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QPointer>
#include <QMap>

class QGraphicsWidget;
class PanParameters;

/*!
 * \brief MImSnapshotPixmapItem is a snapshot which is grabbed from the screen
 * rectangle, or wigets, or pixmaps.
 */
class MImSnapshotPixmapItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    MImSnapshotPixmapItem(const QPixmap &pixmap, QGraphicsItem* parent = 0);
    explicit MImSnapshotPixmapItem(QGraphicsItem *parent = 0);

    virtual ~MImSnapshotPixmapItem();

    //! Grabs the snapshot from screen rectangle.
    void grabScreen(const QRect& rect);

    //! Grabs the snapshot from widgets.
    void grabWidgets(const QList<QPointer<QGraphicsWidget> > &widgets);

    //! Grabs the snapshot from pimaps
    void grabPixmaps(const QMap<QPixmap*, QPoint > &pixmaps);

    /*!
     * \brief Connects to a PanParameter object
     *
     * Connects the updating (position, scale, opacity) slots with signals
     * from \a parametersObj.
     */
    void connectPanParameters(PanParameters *parametersObj);

public slots:
    //! Updates the positions.
    void updatePos(const QPointF &pos);

    //! Updates the opacity.
    void updateOpacity(qreal opacity);

    //! Updates the scale.
    void updateScale(qreal scale);

private:
    QPointer<PanParameters> mParameters;
    Q_DISABLE_COPY(MImSnapshotPixmapItem)
};

#endif
