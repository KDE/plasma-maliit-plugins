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

    //! \brief Request \a reactionMapUpdateNeeded to be emitted.
    void requestReactionMapUpdate();

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

    //! \brief Emitted whenever reaction map update is needed.
    //!
    //! \a regionChanged emission implies \a reactionMapUpdateNeeded emission, but not the
    //! other way around.
    void reactionMapUpdateNeeded();

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
