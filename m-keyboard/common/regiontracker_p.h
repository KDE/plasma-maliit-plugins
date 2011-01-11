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

#ifndef REGIONTRACKER_P_H
#define REGIONTRACKER_P_H

#include <QObject>
#include <QRegion>
#include <QMap>

class RegionTracker;
class QGraphicsWidget;

class RegionStore : public QObject
{
    Q_OBJECT

public:
    RegionStore();

    void addWidget(const QObject &widget);
    void enable(bool newEnabled, bool flush);

    void handleDestroy(const QObject &widget);
    void handleGeometryChange(const QObject &widget, const QRegion &region);

signals:
    //! \brief Combined region of stored widgets has changed
    void regionChanged(const QRegion &region);

private:
    //! \return Union of all regions in \a regions
    QRegion combineRegions() const;

    //! \brief Emit \a regionChanged if enabled and dirty
    void maybeNotify();

public:
    //! The region emitted the last time.
    QRegion lastRegion;
    bool dirty;

private:
    bool enabled;

    //! QObject -> Region mapping for widget regions
    typedef QMap<const QObject *, QRegion> RegionMap;

    //! Regions of widgets
    RegionMap regions;
};


class RegionTrackerPrivate : public QObject
{
    Q_OBJECT
    friend class RegionTracker;

public:
    RegionTrackerPrivate();

private slots:
    void handleDestroy(QObject *object);
    void handleGeometryChange();
    void handleVisibilityChange();

private:
    //! \brief Common work of RegionTracker::addRegion and RegionTracker::addInputMethodArea
    void addWidgetCommon(const QGraphicsWidget &widget);
    void changeGeometry(const QGraphicsWidget &widget);

private:
    bool enabled;

    //! Regions of widgets
    RegionStore widgetRegions;

    //! Regions of widgets that affect the input method area
    RegionStore inputMethodAreaWidgetRegions;

private:
    Q_DISABLE_COPY(RegionTrackerPrivate)
};

#endif
