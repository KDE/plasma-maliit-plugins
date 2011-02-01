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

#ifndef REACTIONMAPPAINTER_P_H
#define REACTIONMAPPAINTER_P_H

#include <QObject>
#include <QTimer>
#include <QVector>

class ReactionMapPainter;
class ReactionMapPaintable;

class ReactionMapPainterPrivate : public QObject
{
    Q_OBJECT
    friend class ReactionMapPainter;

public:
    ReactionMapPainterPrivate();

private:
    //! \brief Add \a widget to the reaction map painter
    void addWidget(ReactionMapPaintable &widget);

    //! \brief Remove \a widget from the reaction map painter
    void removeWidget(const ReactionMapPaintable &widget);

private slots:
    //! \brief Clear the reaction maps
    void clear();

    //! \brief Request repainting the reaction maps
    void requestRepaint();

    //! \brief Repaint the reaction maps
    void repaint();

private:
    //! List of paintable widgets
    QVector<ReactionMapPaintable*> widgets;
    //! Timer for repainting
    QTimer repaintTimer;

    Q_DISABLE_COPY(ReactionMapPainterPrivate)
};

#endif
