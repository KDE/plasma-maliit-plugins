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

#ifndef EXTENDEDKEYS_H
#define EXTENDEDKEYS_H

#include <mimoverlay.h>
#include <mimkeyarea.h>
#include <mimabstractkeyarea.h>
#include <MTheme>
#include <QSize>
#include <QPointer>
#include <memory>

#include "reactionmappaintable.h"

class MagnifierHost;
class MImAbstractKey;
class QWidget;
class QPainter;
class QStyleOptionGraphicsItem;

//! \internal
//! \brief Allows access to protected areas of SingleWidgetButtonArea
class ExtendedKeysArea
    : public MImKeyArea
{
public:
    explicit ExtendedKeysArea(const LayoutData::SharedLayoutSection &section,
                              QGraphicsWidget *parent)
        : MImKeyArea(section, parent)
    {
        // Used by MATTI:
        setObjectName("VirtualKeyboardExtendedArea");
    }

    virtual ~ExtendedKeysArea()
    {}


    // Used for event forwarding, to simulate "touch event grabbing"
    virtual bool event(QEvent *ev)
    {
        return MImKeyArea::event(ev);
    }
};
//! \internal_end

class ExtendedKeys
    : public MImOverlay, public ReactionMapPaintable
{
    Q_OBJECT

private:
    //! host allows interaction with other Magnifier plugin parts.
    MagnifierHost *host;

    //! mainArea allows interaction with VKB.
    QPointer<MImAbstractKeyArea> mainArea;

    //! The actual widget showing the extended keys.
    std::auto_ptr<ExtendedKeysArea> extKeysArea;

    //! Used for non-multitouch mode
    bool hideOnNextMouseRelease;

public:
    explicit ExtendedKeys(MagnifierHost *host,
                          MImAbstractKeyArea *mainArea);
    virtual ~ExtendedKeys();

    /*! \reimp */
    bool isPaintable() const;
    bool isFullScreen() const;
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);
    /*! \reimp_end */

public slots:
    //! \brief Creates (and shows) a KeyButtonArea from labels.
    //! \param origin Usually the center of the active button in the main key area,
    //!        used to position the new extended area.
    //! \param tappedScenePos Position where finger was tapped, in scene coordinates.
    //!        Used to apply initial press on the new extended area widget.
    //! \param labels The labels to use for the buttons.
    void showExtendedArea(const QPointF &origin,
                          const QPointF &tappedScenePos,
                          const QString &labels);

protected:
    //! \reimp
    virtual QVariant itemChange(GraphicsItemChange change,
                                const QVariant &value);
    virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
    //! \reimp_end
};

#endif // EXTENDEDKEYS_H
