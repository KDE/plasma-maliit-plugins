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

#ifndef REGIONTRACKER_P_H
#define REGIONTRACKER_P_H

#include <QObject>
#include <QRegion>
#include <QMap>
#include <QMultiMap>

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
    void handleProxyDestroyed(QObject *object);
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

    //! Widget A -> B+ mapping where changes to geometry of A may affect the
    //! geometry of B, even if B.geometryChanged() is not emitted.
    typedef QMultiMap<const QObject *, const QObject *> GeometryProxyMap;
    GeometryProxyMap geometryProxies;

private:
    Q_DISABLE_COPY(RegionTrackerPrivate)
};

#endif
