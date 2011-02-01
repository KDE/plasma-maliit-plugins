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

#ifndef REACTIONMAPPAINTER_H
#define REACTIONMAPPAINTER_H

#include <QObject>

class ReactionMapPainterPrivate;
class ReactionMapPaintable;

//! \brief A class that does the job of the centralised reaction map painting.
//!
//! This class is public and can be used by pop-up plug-ins.
class ReactionMapPainter : public QObject
{
    Q_OBJECT

    //! \brief Constructor.
    ReactionMapPainter();

public:
    //! Destructor
    virtual ~ReactionMapPainter();

    //! \brief Get singleton instance
    //! \return singleton instance
    static ReactionMapPainter &instance();

    //! \brief Create singleton
    static void createInstance();

    //! \brief Destroy singleton
    static void destroyInstance();

    //! \brief Add \a widget to the reaction map painter
    void addWidget(ReactionMapPaintable &widget);

    //! \brief Remove \a widget from the reaction map painter
    void removeWidget(const ReactionMapPaintable &widget);

public slots:
    //! \brief Clear the reaction maps
    void clear();

    //! \brief Repaint the reaction maps
    void repaint();
private:
    //! Singleton instance
    static ReactionMapPainter *singleton;

    ReactionMapPainterPrivate *const d_ptr;

    Q_DECLARE_PRIVATE(ReactionMapPainter)
};

inline ReactionMapPainter &ReactionMapPainter::instance()
{
    Q_ASSERT(singleton);
    return *singleton;
}

#endif
