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

#ifndef LAYOUTPANNER_H
#define LAYOUTPANNER_H

#include "pangesture.h"
#include "layoutpannerstyle.h"
#include "reactionmappaintable.h"

#include <MStylableWidget>
#include <QPoint>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QPointer>

class QPixmap;
class QGraphicsWidget;
class QGraphicsSceneMouseEvent;
class MReactionMap;
class PanParameters;
class MImSnapshotPixmapItem;
class Notification;
class NotificationArea;

/*!
 * \brief LayoutPanner is used to play panning animation during switching layouts.
 * 
 *  LayoutPanner plays panning animation according pan gesture and shows notification
 *  area.
 */
class LayoutPanner : public MStylableWidget, public ReactionMapPaintable
{
    Q_OBJECT
    /*!
     * panningPosition defines the position of panning widgets.
     * Change this property to update positions for layouts and notifications.
     */
    Q_PROPERTY(QPoint panningPosition READ panningPosition WRITE setPanningPosition)
    /*!
     * layoutProgress defines current layouts panning progress.
     * Change this property to update positions for layouts.
     * */
    Q_PROPERTY(qreal layoutsProgress READ layoutsProgress WRITE setLayoutsProgress)
    /*!
     * notificationsProgress defines current notifications panning progress.
     * Change this property to update positions for notifications.
     */
    Q_PROPERTY(qreal notificationsProgress READ notificationsProgress WRITE setNotificationsProgress)

public:
    virtual ~LayoutPanner();

    //! Returns the current instance, or 0 if none.
    static LayoutPanner &instance();

    //! Creates singleton
    static void createInstance(QGraphicsWidget *parent);

    //! Destroies singleton
    static void destroyInstance();

    //! Enables or disables panning
    void setPanEnabled(bool enable);

    //! Returns true if pan is enabled.
    bool isPanEnabled() const;

    //! Tries pan to \a direction from \a startPos.
    void tryPan(PanGesture::PanDirection direction, const QPoint& startPos);

    //! Adds \a widget as outgoing widget.
    void addOutgoingWidget(QGraphicsWidget *widget);

    //! Clears cached outgoing widgets.
    void clearOutgoingWidgets();

    //! Adds \a widget as incoming widget for pan of \a direction.
    void addIncomingWidget(PanGesture::PanDirection direction, QGraphicsWidget *widget);

    //! Clears cached incoming widgets.
    void clearIncomingWidgets(PanGesture::PanDirection direction);

    //! Grabs the cached incoming widgets to a snapshot.
    void grabIncomingSnapshot();

    //! Prepares the oritation change.
    void prepareOrientationChange();

    //! Finalizes the oritation change.
    void finalizeOrientationChange();

    /*!
     * \brief Add shared pixmap as snapshot.
     * \param pixmap The shared pixmap.
     * \param position The scene coordinate position.
     *
     * The \a pixmap will be grab to the snapshot which is shown on both outgoing
     * and incoming layouts.
     */
    void addSharedPixmap(QPixmap *pixmap, const QPoint &position);

    //! Returns true if it is during switching plugin.
    bool isSwitchingPlugin() const;

    //! Sets the state of switching plugin to \a flag.
    void setSwitchingPlugin(bool flag);

    //! Returns true if it is during panning layouts.
    bool isPanningLayouts() const;

    //! Sets the title for the outgoing layout.
    void setOutgoingLayoutTitle(const QString &title);

    //! Sets the title for the incoming layout of \a direction.
    void setIncomingLayoutTitle(PanGesture::PanDirection direction, const QString &title);

    /*! \reimp */
    bool isPaintable() const;
    /*! \reimp_end */

    //! Cancels the panning.
    void cancel();

protected:
    explicit LayoutPanner(QGraphicsWidget *parent);

    //!reimp
    virtual void applyStyle();
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget * widget = 0);
    //! reimp_end

signals:
    /*!
     * \brief Emitted when request preparing layout pan.
     * 
     * startPos in scene coordinates
     */
    void preparingLayoutPan(PanGesture::PanDirection direction,
                                        const QPoint &startPos);

    /*!
     * \brief Emitted when layout pan is finished.
     */
    void layoutPanFinished(PanGesture::PanDirection result);

public slots:
    //! \brief Paint reaction map.
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

private slots:
    void onPanningAnimationFinished();

    void onCatchingUpAnimationFinished();

private:
    const QPoint &panningPosition() const;
    void setPanningPosition(const QPoint &pos);
    void goToPanningPosition(const QPoint &start, const QPoint &end);
    qreal layoutsProgress() const;
    void setLayoutsProgress(qreal progress);
    qreal notificationsProgress() const;
    void setNotificationsProgress(qreal progress);
    void prepare();
    /*!
     * \brief finalize the panning switch.
     */
    void finalize();
    void updateDimmingPixmapItem();
    void preparePanningItems();
    int distance() const;
    void reset();

    bool mPanEnabled;
    QPoint startPos;
    QPoint currentPos;
    QPoint lastMousePos;
    PanGesture::PanDirection direction;
    QPropertyAnimation panningAnimation;
    QParallelAnimationGroup catchingUpAnimationGroup;
    QList<QPointer<QGraphicsWidget> > outgoingWidgets;
    QList<QPointer<QGraphicsWidget> > leftIncomingWidgets;
    QList<QPointer<QGraphicsWidget> > rightIncomingWidgets;
    QMap<QPixmap*, QPoint > sharedPixmapMap;
    PanGesture::PanDirection result;
    bool mPluginSwitching;
    MImSnapshotPixmapItem *outgoingLayoutItem;
    MImSnapshotPixmapItem *incomingLayoutItem;
    MImSnapshotPixmapItem *leftLayoutItem;
    MImSnapshotPixmapItem *rightLayoutItem;
    MImSnapshotPixmapItem *maskPixmapItem;
    MImSnapshotPixmapItem *sharedPixmapItem;
    NotificationArea *notificationArea;
    PanParameters *outgoingLayoutParameters;
    PanParameters *incomingLayoutParameters;
    PanParameters *foregroundMaskPanParameters;
    qreal mLayoutsProgress;
    qreal mNotificationsProgress;
    static LayoutPanner *sharedInstance;

    M_STYLABLE_WIDGET(LayoutPannerStyle)
};

inline LayoutPanner &LayoutPanner::instance()
{
    Q_ASSERT(sharedInstance);
    return *sharedInstance;
}

#endif
