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

#include "regiontracker.h"
#include "regiontracker_p.h"

#include <mabstractinputmethodhost.h>

#include <QDebug>
#include <QGraphicsWidget>


RegionStore::RegionStore()
    : lastRegion(-2, -2, -1, -1), // unrealistic region
      dirty(false),
      enabled(true)
{
}

void RegionStore::addWidget(const QObject &widget)
{
    regions[&widget] = QRect();
}

void RegionStore::handleGeometryChange(const QObject &widget, const QRegion &region)
{
    if (!regions.contains(&widget)) {
        return;
    }
    regions[&widget] = region;
    dirty = true;
    maybeNotify();
}

void RegionStore::maybeNotify()
{
    if (enabled && dirty) {
        const QRegion newRegion(combineRegions());
        if (newRegion != lastRegion) {
            lastRegion = newRegion;
            emit regionChanged(newRegion);
        }
        dirty = false;
    }
}

QRegion RegionStore::combineRegions() const
{
    QRegion combinedRegion;
    foreach (const QRegion &partialRegion, regions) {
        combinedRegion |= partialRegion;
    }

    return combinedRegion;
}

void RegionStore::handleDestroy(const QObject &widget)
{
    regions.remove(&widget);
    dirty = true;
    maybeNotify();
}

void RegionStore::enable(bool newEnabled, bool flush)
{
    enabled = newEnabled;
    if (flush) {
        maybeNotify();
    }
}


// RegionTrackerPrivate......................................................

RegionTrackerPrivate::RegionTrackerPrivate()
    : enabled(true)
{
}

void RegionTrackerPrivate::addWidgetCommon(const QGraphicsWidget &widget)
{
    changeGeometry(widget);
    connect(&widget, SIGNAL(geometryChanged()),
            this, SLOT(handleGeometryChange()), Qt::UniqueConnection);
    connect(&widget, SIGNAL(destroyed(QObject *)),
            this, SLOT(handleDestroy(QObject *)), Qt::UniqueConnection);
    connect(&widget, SIGNAL(visibleChanged()),
            this, SLOT(handleVisibilityChange()), Qt::UniqueConnection);
}

void RegionTrackerPrivate::handleGeometryChange()
{
    const QGraphicsWidget *widget(dynamic_cast<const QGraphicsWidget *>(QObject::sender()));
    if (!widget->isVisible()) {
        return;
    }
    changeGeometry(*widget);
}

void RegionTrackerPrivate::changeGeometry(const QGraphicsWidget &widget)
{
    const QRegion region(widget.isVisible() ? widget.mapRectToScene(widget.rect()).toRect()
                         : QRect());
    widgetRegions.handleGeometryChange(widget, region);
    inputMethodAreaWidgetRegions.handleGeometryChange(widget, region);
}


void RegionTrackerPrivate::handleDestroy(QObject *widget)
{
    widgetRegions.handleDestroy(*widget);
    inputMethodAreaWidgetRegions.handleDestroy(*widget);
}

void RegionTrackerPrivate::handleVisibilityChange()
{
    const QGraphicsWidget *widget(dynamic_cast<const QGraphicsWidget *>(QObject::sender()));
    changeGeometry(*widget);
}


// RegionTracker.............................................................

RegionTracker *RegionTracker::Instance = 0;


RegionTracker::RegionTracker()
    : d_ptr(new RegionTrackerPrivate)
{
    Q_D(RegionTracker);

    connect(&d->widgetRegions, SIGNAL(regionChanged(const QRegion &)),
            this, SIGNAL(regionChanged(const QRegion &)));
    connect(&d->inputMethodAreaWidgetRegions, SIGNAL(regionChanged(const QRegion &)),
            this, SIGNAL(inputMethodAreaChanged(const QRegion &)));
    connect(this, SIGNAL(regionChanged(const QRegion &)),
            this, SIGNAL(reactionMapUpdateNeeded()));
}

RegionTracker::~RegionTracker()
{
    delete d_ptr;
}


void RegionTracker::createInstance()
{
    Q_ASSERT(!Instance);
    if (!Instance) {
        Instance = new RegionTracker();
    }
}

void RegionTracker::destroyInstance()
{
    Q_ASSERT(Instance);
    delete Instance;
    Instance = 0;
}


void RegionTracker::addRegion(const QGraphicsWidget &widget)
{
    Q_D(RegionTracker);
    d->widgetRegions.addWidget(widget);
    d->addWidgetCommon(widget);
}

void RegionTracker::addInputMethodArea(const QGraphicsWidget &widget)
{
    Q_D(RegionTracker);
    d->inputMethodAreaWidgetRegions.addWidget(widget);
    d->addWidgetCommon(widget);
}

bool RegionTracker::enableSignals(bool newEnabled, bool flush)
{
    Q_D(RegionTracker);
    const bool wasEnabled(d->enabled);
    d->enabled = newEnabled;
    d->widgetRegions.enable(newEnabled, flush);
    d->inputMethodAreaWidgetRegions.enable(newEnabled, flush);
    return wasEnabled;
}

void RegionTracker::requestReactionMapUpdate()
{
    emit reactionMapUpdateNeeded();
}

void RegionTracker::sendInputMethodAreaEstimate(const QRegion &region)
{
    Q_D(RegionTracker);
    d->inputMethodAreaWidgetRegions.lastRegion = region;
    d->inputMethodAreaWidgetRegions.dirty = false;
    emit inputMethodAreaChanged(region);
}

void RegionTracker::sendRegionEstimate(const QRegion &region)
{
    Q_D(RegionTracker);
    d->widgetRegions.lastRegion = region;
    d->widgetRegions.dirty = false;
    emit regionChanged(region);
}
