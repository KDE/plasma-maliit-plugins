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

#ifndef MAGNIFIER_H
#define MAGNIFIER_H

#include "mimoverlay.h"

#include <QObject>
#include <QPointF>
#include <QString>
#include <QMargins>

class MagnifierHost;
class QPainter;

//! Keyboard button magnifier.
//! Shows a key label on top of the current key, magnified.
class Magnifier: public MImOverlay
{
    Q_OBJECT
    Q_DISABLE_COPY(Magnifier);

private:
    //! Position of popup's top left corner in its local coordinates.
    QPointF topLeft;

    //! Rectangle of the popup text area in its local coordinates.
    QRectF textArea;

    //! Label is shown inside the magnifier.
    QString label;

    //! Host allows interaction with other Magnifier plugin parts.
    MagnifierHost *host;

    //! "Parent" item for geometry calculation.
    //! Real parentItem() is used only for Z-ordering,
    //! and position is handled in coordinates of geometryParentItem
    QGraphicsItem *geometryParentItem;

    //! Safety margins (wrt. parent item) of the Magnifier.
    QMargins safetyMargins;

public:
    //! Constructor
    //! \param host the MagnifierHost
    //! \param parent the parent item
    explicit Magnifier(MagnifierHost *host,
                       QGraphicsItem *parent = 0);

    //! Destructor
    virtual ~Magnifier();

    //! Sets up Magnifier
    virtual void setup();

    //! \reimp
    virtual void paint(QPainter *,
                       const QStyleOptionGraphicsItem *,
                       QWidget* = 0);

    virtual QRectF boundingRect() const;
    //! \reimp_end

    //! Sets the Magnifier's position
    //! \param keyPos the position of the magnified key
    //! \param scenePos not used
    //! \param keySize the size of the magnified key
    virtual void updatePos(const QPointF &keyPos,
                           const QPoint &scenePos,
                           const QSize &keySize);

    //! Sets the label for the Magnifier
    //! \param label the label
    virtual void setLabel(const QString &label);

    //! Sets the safety margins (i.e., constraints the valid area for the
    //! Magnifier's position.
    //! \param safetyMargins the safety margins. Only left and right margins
    //!        should be used, top and bottom should be set to -1.
    virtual void setSafetyMargins(const QMargins &safetyMargins);
};

#endif

