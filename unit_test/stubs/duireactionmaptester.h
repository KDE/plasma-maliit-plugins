/* * This file is part of dui-keyboard *
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


#ifndef DUIREACTIONMAPTESTER_H
#define DUIREACTIONMAPTESTER_H

#include "duireactionmap_stub.h"

#include <QDebug>
#include <QGraphicsItem>

#include "keybuttonarea.h"

#include <DuiButton>
#include <DuiDeviceProfile>
#include <DuiLabel>

#include <string.h>

namespace
{
    const int ReactionMapWidth = 216;
    const int ReactionMapHeight = 120;
}

class DuiReactionMapTester : public DuiReactionMapStub
{
public:
    enum ReactionColorValue {
        Inactive,
        Transparent,
        ReactivePressRelease,
        NumReactiveColorValues,
        InvalidReactiveColor
    };

    DuiReactionMapTester();
    virtual ~DuiReactionMapTester();

    //! \reimp
    virtual void duiReactionMapConstructor(QWidget *topLevelWidget, const QString &appIdentifier, QObject *parent);
    virtual void duiReactionMapDestructor();

    virtual DuiReactionMap *instance(QWidget *anyWidget);
    virtual void setInactiveDrawingValue();
    virtual void setReactiveDrawingValue();
    virtual void setTransparentDrawingValue();
    virtual void setDrawingValue(const QString &pressFeedback, const QString &releaseFeedback);
    virtual QTransform transform() const;
    virtual void setTransform(QTransform transform);
    virtual void setTransform(const QGraphicsItem *item, const QGraphicsView *view);
    virtual void fillRectangle(int x, int y, int width, int height);
    virtual void fillRectangle(const QRect &rectangle);
    virtual void fillRectangle(const QRectF &rectangle);
    virtual void fillRectangle(const QRect &rectangle,
                               const QString &pressFeedback,
                               const QString &releaseFeedback);
    virtual void fillRectangle(const QRectF &rectangle,
                               const QString &pressFeedback,
                               const QString &releaseFeedback);
    virtual int width() const;
    virtual int height() const;
    virtual void clear();
    //! \reimp_end

    //! \brief Checks that every DuiButton child of \a item has reactive area drawn on it.
    bool testChildButtonReactiveAreas(const QGraphicsView *view, const QGraphicsItem *item);

    /*! \brief Checks whether correct reaction color is drawn at different grid points of the view.
     *  \param view The QGraphicsView on which the grid is applied.
     *  \param gridSize Number of grid points horizontally and vertically
     *  \param minCoverage Minnimum percentage of unambiguous grid points to check. Otherwise fail.
     *  \param region Optional region which tells whether or not a grid point belongs to visible widget.
     */
    bool testReactionMapGrid(const QGraphicsView *view, const int gridSize, const int minCoverage, const QRegion &region);
    bool isReactiveArea(const QRectF &rectangle) const;
    ReactionColorValue colorAt(const QPointF &pos) const;
    ReactionColorValue colorAt(const QPointF &pos, int margin, bool *ambiguous) const;

    void tmpDump() const;

    int screenWidth() const;
    int screenHeight() const;

protected:
    void drawRect(const QRect &rectangle);
    QTransform viewToReactionMap() const;
    QTransform itemToReactionMap(const QGraphicsItem *item) const;
    bool isInsideReactionMap(const QPoint &point) const;
    bool isInsideReactionMap(const QRect &rect) const;
    QRect clip(const QRect &rect) const;

protected:
    struct InstanceData {
        uchar bitmap[ReactionMapWidth *ReactionMapHeight];
        QTransform transform;
        uchar currentColor;
    };

    //! Only one DuiReactionMap instance for now.
    static InstanceData data;
    static DuiReactionMap *globalReactionMap;
};

DuiReactionMapTester::InstanceData DuiReactionMapTester::data;
DuiReactionMap *DuiReactionMapTester::globalReactionMap = 0;

DuiReactionMapTester::DuiReactionMapTester()
{
    data.currentColor = Inactive;
}

DuiReactionMapTester::~DuiReactionMapTester()
{
    delete globalReactionMap;
    globalReactionMap = 0;
}

void DuiReactionMapTester::duiReactionMapConstructor(QWidget */*topLevelWidget*/, const QString& /*appIdentifier*/, QObject */*parent*/)
{
    // Multiple instances not supported by stub, at least yet.
    Q_ASSERT(!globalReactionMap);
}

void DuiReactionMapTester::duiReactionMapDestructor()
{
}

DuiReactionMap *DuiReactionMapTester::instance(QWidget */*anyWidget*/)
{
    if (!globalReactionMap) {
        globalReactionMap = new DuiReactionMap(0);
    }

    return globalReactionMap;
}

void DuiReactionMapTester::setInactiveDrawingValue()
{
    data.currentColor = Inactive;
}

void DuiReactionMapTester::setReactiveDrawingValue()
{
    data.currentColor = ReactivePressRelease;
}

void DuiReactionMapTester::setTransparentDrawingValue()
{
    data.currentColor = Transparent;
}

void DuiReactionMapTester::setDrawingValue(const QString &pressFeedback, const QString &/*releaseFeedback*/)
{
    uchar color;

    // Support only Inactive/Inactive, Transparent/Transparent, or Press/Release
    if (pressFeedback == DuiReactionMap::Inactive) {
        color = Inactive;
    } else if (pressFeedback == DuiReactionMap::Transparent) {
        color = Transparent;
    } else {
        color = ReactivePressRelease;
    }

    data.currentColor = color;
}

QTransform DuiReactionMapTester::transform() const
{
    return data.transform;
}

void DuiReactionMapTester::setTransform(QTransform transform)
{
    data.transform = transform;
}

void DuiReactionMapTester::setTransform(const QGraphicsItem *item, const QGraphicsView */*view*/)
{
    setTransform(itemToReactionMap(item));
}

void DuiReactionMapTester::fillRectangle(int x, int y, int width, int height)
{
    fillRectangle(QRect(x, y, width, height));
}

void DuiReactionMapTester::fillRectangle(const QRect &rectangle)
{
    drawRect(transform().mapRect(rectangle));
}

void DuiReactionMapTester::fillRectangle(const QRectF &rectangle)
{
    drawRect(transform().mapRect(rectangle).toRect());
}

void DuiReactionMapTester::fillRectangle(const QRect &rectangle, const QString &pressFeedback, const QString &releaseFeedback)
{
    uchar prevColor = data.currentColor;
    setDrawingValue(pressFeedback, releaseFeedback);
    drawRect(transform().mapRect(rectangle));
    data.currentColor = prevColor;
}

void DuiReactionMapTester::fillRectangle(const QRectF &rectangle, const QString &pressFeedback, const QString &releaseFeedback)
{
    uchar prevColor = data.currentColor;
    setDrawingValue(pressFeedback, releaseFeedback);
    drawRect(transform().mapRect(rectangle).toRect());
    data.currentColor = prevColor;
}

int DuiReactionMapTester::width() const
{
    return ReactionMapWidth;
}

int DuiReactionMapTester::height() const
{
    return ReactionMapHeight;
}

void DuiReactionMapTester::clear()
{
    // Clear means inactive
    memset(&data.bitmap[0], Inactive, ReactionMapWidth * ReactionMapHeight);
}

bool DuiReactionMapTester::testChildButtonReactiveAreas(const QGraphicsView *view, const QGraphicsItem *item)
{
    // This test does not walk through every button in vkb because most of them are not DuiButtons anymore.
    // However, it does check the few buttons that are so this is still useful. Also, we still have the
    // grid test to cover all buttons.
    bool success = true;

    if (item->isVisible()) {
        if (dynamic_cast<const DuiButton *>(item)) {
            setTransform(item, view);
            success = isReactiveArea(item->boundingRect().toRect());

            if (!success) {
                qDebug() << "Button with scene rect " << item->mapRectToScene(item->boundingRect())
                         << " and label " << static_cast<const DuiButton *>(item)->text()
                         << " has incomplete reactive area.";
            }
        } else {
            const QList<QGraphicsItem *> children = item->childItems();
            foreach(const QGraphicsItem * child, children) {
                if (!testChildButtonReactiveAreas(view, child)) {
                    success = false;
                    break;
                }
            }
        }
    }
    return success;
}

bool DuiReactionMapTester::testReactionMapGrid(const QGraphicsView *view,
                                               const int gridSize,
                                               const int minCoverage,
                                               const QRegion &region)
{
    const int verticalIncrement = screenHeight() / gridSize;
    const int horizontalIncrement = screenWidth() / gridSize;

    int skipCount = 0;

    setTransform(viewToReactionMap());

    for (int y = 0; y < screenHeight(); y += verticalIncrement) {
        for (int x = 0; x < screenWidth(); x += horizontalIncrement) {

            // We don't currently have any transformation between view and scene so (x,y) is directly scene coordinates.
            const QPoint scenePoint = QPoint(x,y);

            // We use QGraphicsScene::items() QRectF version because it respects IntersectsItemBoundingRect flag.
            // QPointF version of it will call QGraphicsItem::contains() which in turn uses QGraphicsItem::shape().
            // We want to use IntersectsItemBoundingRect because bounding rectangle contains reactive margins, shape may not.
            const QList<QGraphicsItem *> items = view->scene()->items(QRectF(scenePoint, QSizeF(1, 1)), Qt::IntersectsItemBoundingRect, Qt::DescendingOrder);
            const QGraphicsItem *item = items.isEmpty() ? 0 : items.first();

            // Get color value of reaction map at viewPos.
            bool ambiguous;
            const int margin = 1 + screenWidth() / width(); // max coordinate conversion error + 1 just to be safe
            ReactionColorValue color = colorAt(scenePoint, margin, &ambiguous);

            if (ambiguous) {
                ++skipCount;
                continue;
            }

            if (item && !region.isEmpty() && !region.contains(scenePoint)) {
                // We are not interested in this scene item.
                // It's probably the parent scene window.
                item = 0;
            }

            // If we hit DuiLabel take the parent item which is DuiButton
            if (dynamic_cast<const DuiLabel *>(item)) {
                item = item->parentItem();
                Q_ASSERT(dynamic_cast<const DuiButton *>(item));
            }

            DuiReactionMapTester::ReactionColorValue expectedColor;

            const KeyButtonArea *kba = dynamic_cast<const KeyButtonArea*>(item);

            // Choose expected color.
            // The expected color is based on which item was under current grid point.
            if (!item) {
                // No scene item -> we should be transparent
                expectedColor = Transparent;
            } else if (dynamic_cast<const DuiButton *>(item)
                       || (kba && kba->keyAt(kba->mapFromScene(scenePoint).toPoint()))) {
                // Buttons should always have reactive color.
                expectedColor = ReactivePressRelease;
            } else {
                // Item is part of SymbolView's area but not a button -> should be inactive
                expectedColor = Inactive;
            }

            if (color != expectedColor) {
                qDebug() << "Wrong reactionmap color at scene pos " << scenePoint
                         << "; expected " << expectedColor << ", got " << color;
                if (item) {
                    qDebug() << " item boundaries at failed pos: " << item->mapRectToScene(item->boundingRect());

                    const QObject *qobj = dynamic_cast<const QObject*>(item);
                    if (qobj) {
                        qDebug() << " Item class name: " << qobj->metaObject()->className();
                    }
                }

                return false;
            }
        }
    }

    const int gridCoverage = ((qreal)(gridSize * gridSize - skipCount) / (qreal)(gridSize * gridSize)) * 100.0;
    qDebug() << "Grid coverage: " << gridCoverage << "%";

    return (gridCoverage >= minCoverage);
}


bool DuiReactionMapTester::isReactiveArea(const QRectF &rectangle) const
{
    // Map rect center point to reaction map coordinates.
    QRect mapRect = clip(transform().mapRect(rectangle).toRect());

    const int margin = (int)(0.01 * ReactionMapWidth);
    mapRect.adjust(margin, margin, -margin, -margin);
    if (!mapRect.isValid()) {
        // Adjusted too much, just check the middle point.
        return (colorAt(rectangle.center()) == ReactivePressRelease);
    }

    for (int y = mapRect.y(); y < mapRect.bottom(); ++y) {
        for (int x = mapRect.x(); x < mapRect.right(); ++x) {
            const uchar color = data.bitmap[y * width() + x];
            if (color != (uchar)ReactivePressRelease) {
                return false;
            }
        }
    }

    return true;
}

DuiReactionMapTester::ReactionColorValue DuiReactionMapTester::colorAt(const QPointF &pos) const
{
    uchar color = InvalidReactiveColor;
    QPoint mapPos = transform().map(pos).toPoint();

    if (isInsideReactionMap(mapPos)) {
        color = data.bitmap[mapPos.y() * width() + mapPos.x()];
        Q_ASSERT(color < NumReactiveColorValues);
    }
    return (ReactionColorValue)color;
}

DuiReactionMapTester::ReactionColorValue DuiReactionMapTester::colorAt(const QPointF &pos, int margin, bool *ambiguous) const
{
    Q_ASSERT(ambiguous);
    *ambiguous = false;

    if (margin == 0) {
        return colorAt(pos);
    }

    uchar color = InvalidReactiveColor;

    const QRectF marginBox = QRectF(QPoint(-margin, -margin), QPoint(margin, margin)).translated(pos);
    QRect area = transform().mapRect(marginBox).toRect();

    if (isInsideReactionMap(area)) {
        const uchar *rowptr = &data.bitmap[area.y() * width()];
        for (int y = area.y(); !(*ambiguous) && y < area.y() + area.height(); ++y) {

            for (int x = area.x(); x < area.x() + area.width(); ++x) {
                const uchar newColor = *(rowptr + x);
                if (color == InvalidReactiveColor) {
                    // First time here, set the color which should not change if there's no ambiguity.
                    color = newColor;
                } else if (color != newColor) {
                    *ambiguous = true;
                    color = InvalidReactiveColor;
                    break;
                }
            }
            rowptr += width();
        }
    } else {
        *ambiguous = true;
    }
    Q_ASSERT(!(*ambiguous && (color != InvalidReactiveColor)));
    return (ReactionColorValue)color;
}

int DuiReactionMapTester::screenWidth() const
{
    return DuiDeviceProfile::instance()->resolution().width();
}

int DuiReactionMapTester::screenHeight() const
{
    return DuiDeviceProfile::instance()->resolution().height();
}

void DuiReactionMapTester::tmpDump() const
{
    // :)

    const int dx = width() / 60;
    const int dy = height() / 60;

    for (int y = 0; y < height(); ++y) {
        if (y % dy) continue;
        for (int x = 0; x < width(); ++x) {
            if (x % dx) continue;
            uchar color = data.bitmap[y * width() + x];
            if (color == 0) {
                printf("I");
            } else if (color == 1) {
                printf("T");
            } else if (color == 2) {
                printf("R");
            } else {
                printf("?");
            }
        }
        printf("\n");
    }
}

void DuiReactionMapTester::drawRect(const QRect &rectangle)
{
    const QRect dstRect = clip(rectangle);

    const int scanlineWidth = width();
    uchar *const rowBegin = &data.bitmap[dstRect.y() * scanlineWidth];
    uchar *const rowEnd = &data.bitmap[(dstRect.y() + dstRect.height()) * scanlineWidth];

    int fillValue = (int)data.currentColor;

    for (uchar *row = rowBegin; row != rowEnd; row += scanlineWidth) {
        uchar *data = row + dstRect.x();
        memset(data, fillValue, dstRect.width());
    }
}

QTransform DuiReactionMapTester::viewToReactionMap() const
{
    QTransform viewToBitmap;
    viewToBitmap.translate(0, 0); // Assume (0,0) offset for view
    viewToBitmap.translate(-0.5, -0.5); // We deal only with integer points & rectangles so effectively this makes floor().
    viewToBitmap.scale((qreal)width() / (qreal)screenWidth(), (qreal)height() / (qreal)screenHeight());
    return viewToBitmap;
}

QTransform DuiReactionMapTester::itemToReactionMap(const QGraphicsItem *item) const
{
    // Assuming identity view transform.
    return (item->sceneTransform() * viewToReactionMap());
}

QRect DuiReactionMapTester::clip(const QRect &rect) const
{
    return (rect & QRect(0, 0, width(), height()));
}

bool DuiReactionMapTester::isInsideReactionMap(const QPoint &point) const
{
    return QRect(0, 0, width(), height()).contains(point);
}

bool DuiReactionMapTester::isInsideReactionMap(const QRect &rect) const
{
    return QRect(0, 0, width(), height()).contains(rect);
}

#endif // DUIREACTIONMAPTESTER_H
