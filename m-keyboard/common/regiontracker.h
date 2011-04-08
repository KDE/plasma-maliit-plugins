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

#ifndef REGIONTRACKER_H
#define REGIONTRACKER_H

#include <QObject>
#include <QRegion>

class QGraphicsWidget;
class RegionTrackerPrivate;

//! \brief A class that automatically tracks total region and input method area covered by
//! widgets registered to it by observing their geometry and visibility changes.
//!
//! Note that widget geometry (position, in practice) changes don't cause
//! children to notify that their geometry has been changed and therefore
//! RegionTracker cannot cover such cases in its current form.
//!
//! This class is public and can be used by popup plugins.
class RegionTracker : public QObject
{
    Q_OBJECT

public:
    //! Destructor
    virtual ~RegionTracker();

    //! \brief Get singleton instance
    //! \return singleton instance
    static RegionTracker &instance();

    //! \brief Create singleton
    static void createInstance();

    //! \brief Destroy singleton
    static void destroyInstance();

    //! \brief Make \a widget part of the tracked screen region
    void addRegion(const QGraphicsWidget &widget);

    //! \brief Make \a widget part of the tracked input method area
    void addInputMethodArea(const QGraphicsWidget &widget);

    //! \brief Enable or disable \a regionChanged and \a inputMethodAreaChanged signals
    //!
    //! If region or input method area is changed while signals are disabled, it is marked
    //! dirty.  When signals are enabled again, dirty region and/or input method area is
    //! signaled if \a flush is true.
    bool enableSignals(bool newEnabled, bool flush = true);

    //! \brief Request given \a region to be emitted via \a inputMethodAreaChanged
    //! regardless of signals being enabled or disabled
    void sendInputMethodAreaEstimate(const QRegion &region);

    //! \brief Request given \a region to be emitted via \a regionChanged
    //! regardless of signals being enabled or disabled
    void sendRegionEstimate(const QRegion &region);

signals:
    //! \brief Screen area covered by widgets registered with \a addRegion was changed
    void regionChanged(const QRegion &region);

    //! \brief Screen area covered by widgets registered with \a addInputMethodArea was changed
    void inputMethodAreaChanged(const QRegion &region);

private:
    //! \brief Constructor.
    RegionTracker();

private:
    //! Singleton instance
    static RegionTracker *Instance;

    RegionTrackerPrivate *const d_ptr;

    Q_DECLARE_PRIVATE(RegionTracker)
};

inline RegionTracker &RegionTracker::instance()
{
    Q_ASSERT(Instance);
    return *Instance;
}

#endif
