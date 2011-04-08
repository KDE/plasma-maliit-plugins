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
    if (!(regions[&widget] ^ region).isEmpty()) {
        regions[&widget] = region;
        dirty = true;
    }
    maybeNotify();
}

void RegionStore::maybeNotify()
{
    if (enabled && dirty) {
        const QRegion newRegion(combineRegions());
        if (!(newRegion ^ lastRegion).isEmpty()) {
            lastRegion = newRegion;
            emit regionChanged(newRegion);
        }
        dirty = false;
    }
}

QRegion RegionStore::combineRegions() const
{
    QRegion combinedRegion;

    for (RegionMap::iterator i(regions.begin()); i != regions.end(); ++i) {
        const QGraphicsWidget &widget(dynamic_cast<const QGraphicsWidget &>(*i.key()));
        const QRegion region(widget.isVisible() ? widget.mapRectToScene(widget.rect()).toRect()
                             : QRect());
        combinedRegion |= region;
        i.value() = region;
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
            this, SLOT(handleGeometryChange()), Qt::QueuedConnection);
    connect(&widget, SIGNAL(destroyed(QObject *)),
            this, SLOT(handleDestroy(QObject *)), Qt::UniqueConnection);
    connect(&widget, SIGNAL(visibleChanged()),
            this, SLOT(handleVisibilityChange()), Qt::QueuedConnection);
}

void RegionTrackerPrivate::handleGeometryChange()
{
    const QGraphicsWidget *widget(dynamic_cast<const QGraphicsWidget *>(QObject::sender()));
    if (!widget || !widget->isVisible()) {
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
    if (!widget) {
        return;
    }
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
