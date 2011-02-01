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

#ifndef REACTIONMAPPAINTABLE_H
#define REACTIONMAPPAINTABLE_H

#include <QObject>

class MReactionMap;
class QGraphicsView;

//! \brief A helper class for reaction map painting to enable signals in ReactionMapPaintable
class ReactionMapPaintableSignaler : public QObject
{
    Q_OBJECT

public:
    //! \brief Emit a request for reaction map repainting
    void emitRequestRepaint();

signals:
    //! \brief Request a reaction map repainting
    void requestRepaint();

    //! \brief Request a reaction map clearing
    void requestClear();
};

//! \brief A class for widgets with reaction map painting
class ReactionMapPaintable
{
public:
    //! \brief Ctor
    ReactionMapPaintable();
    //! \brief Dtor
    virtual ~ReactionMapPaintable();

    //! \brief Paint the reaction map of the widget
    virtual void paintReactionMap(MReactionMap *, QGraphicsView *) = 0;

    //! \brief Is the widget paintable
    virtual bool isPaintable() const = 0;

    //! \brief Is the widget full screen
    virtual bool isFullScreen() const;

    ReactionMapPaintableSignaler signalForwarder;
};

#endif
