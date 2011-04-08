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
