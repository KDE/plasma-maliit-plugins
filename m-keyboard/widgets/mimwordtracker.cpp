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



#include "mimwordtracker.h"
#include "mimcorrectioncandidateitem.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

#include <mplainwindow.h>
#include <mreactionmap.h>
#include <MScalableImage>
#include <MSceneManager>
#include <MScene>
#include <MSceneWindow>
#include <MGConfItem>
#include <float.h>

#include <mwidgetcreator.h>
M_REGISTER_WIDGET_NO_CREATE(MImWordTracker)

namespace
{
    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSetting = "/meegotouch/inputmethods/multitouch/enabled";

    const char * const WordTrackerObjectName = "CorrectionWordTracker";
    const int DefaultShowHideFrames = 100;
    const int DefaultShowHideTime = 400;
    const int DefaultShowHideInterval = 30;

    QRect mapToScreenRect(const QRect &widgetRect)
    {
        if (!widgetRect.isValid()) {
            return QRect();
        }

        M::OrientationAngle angle  = MPlainWindow::instance()->orientationAngle();
        int displayHeight = MPlainWindow::instance()->visibleSceneSize(M::Landscape).height();
        int displayWidth  = MPlainWindow::instance()->visibleSceneSize(M::Landscape).width();

        QRect rect;
        switch (angle) {
            case M::Angle90:
                rect.setRect(displayWidth - widgetRect.y()- widgetRect.height(),
                             widgetRect.x(),
                             widgetRect.height(), widgetRect.width());
                break;
            case M::Angle270:
                rect.setRect(widgetRect.y(),
                             displayHeight - widgetRect.x() - widgetRect.width(),
                             widgetRect.height(), widgetRect.width());
                break;
            case M::Angle180:
                rect.setRect(displayWidth - widgetRect.x() - widgetRect.width(),
                             displayHeight - widgetRect.y() - widgetRect.height(),
                             widgetRect.width(), widgetRect.height());
                break;
            case M::Angle0:
                rect = widgetRect;
                break;
            default:
                qCritical() << __FILE__ << __LINE__ << " Incorrect orientation " << angle;
                rect = QRect();
                break;
        }
        return rect;
    }
};


MImWordTracker::MImWordTracker(MSceneWindow *parentWindow)
    : MStylableWidget(),
      containerWidget(new QGraphicsWidget()),
      mIdealWidth(0),
      candidateItem(new MImCorrectionCandidateItem("", this))
{
    containerWidget->setParentItem(parentWindow);
    this->setParentItem(containerWidget);

    // By default multi-touch is disabled
    setAcceptTouchEvents(MGConfItem(MultitouchSetting).value().toBool());

    setObjectName(WordTrackerObjectName);

    mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
    mainLayout->addItem(candidateItem);
    mainLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(candidateItem, SIGNAL(clicked()), this, SLOT(select()));

    connect(MTheme::instance(), SIGNAL(themeChangeCompleted()),
            this, SLOT(onThemeChangeCompleted()),
            Qt::UniqueConnection);

    setupTimeLine();
    containerWidget->hide();
}

MImWordTracker::~MImWordTracker()
{
    this->setParentItem(0);
    delete containerWidget;
}

void MImWordTracker::setCandidate(const QString &string)
{
    mCandidate = string;
    candidateItem->setTitle(string);

    mIdealWidth = candidateItem->idealWidth();;
    // not less than minimum width
    if (mIdealWidth < minimumSize().width())
        mIdealWidth = minimumSize().width();
    
    mIdealWidth += style()->paddingLeft() + style()->paddingRight()
                   + style()->marginLeft() + style()->marginRight();
    setPreferredWidth(mIdealWidth);
}

QString MImWordTracker::candidate() const
{
    return mCandidate;
}

qreal MImWordTracker::idealWidth() const
{
    return mIdealWidth;
}

qreal MImWordTracker::pointerHeight() const
{
    return (style()->wordtrackerPointerSize().height()
            - style()->wordtrackerPointerOverlap());
}

void MImWordTracker::drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option) const
{
    MStylableWidget::drawBackground(painter, option);
    if (style()->wordtrackerPointerImage()) {
         const QSize pointerSize = style()->wordtrackerPointerSize();
         QRect rect = QRect((idealWidth() - pointerSize.width())/2,
                            style()->wordtrackerPointerOverlap() - pointerSize.height(),
                            pointerSize.width(),
                            pointerSize.height());
         style()->wordtrackerPointerImage()->draw(rect, painter);
    }
}

void MImWordTracker::select()
{
    if (showHideTimeline.state() == QTimeLine::Running) {
        // Ignore select actions during animation.
        return;
    }
    if (!mCandidate.isEmpty()) {
        emit candidateClicked(mCandidate);
    }
}

void MImWordTracker::setupTimeLine()
{
    int showHideFrames = style()->showHideFrames();
    showHideFrames = (showHideFrames > 0) ? showHideFrames : DefaultShowHideFrames;
    int showHideTime = style()->showHideTime();
    showHideTime = (showHideTime > 0) ? showHideTime : DefaultShowHideTime;
    int showHideInterval = style()->showHideInterval();
    showHideInterval = (showHideInterval > 0) ? showHideInterval : DefaultShowHideInterval;

    showHideTimeline.setCurveShape(QTimeLine::EaseInCurve);
    showHideTimeline.setFrameRange(0, showHideFrames);
    showHideTimeline.setDuration(showHideTime);
    showHideTimeline.setUpdateInterval(showHideInterval);
    connect(&showHideTimeline, SIGNAL(frameChanged(int)), this, SLOT(fade(int)), Qt::UniqueConnection);
    connect(&showHideTimeline, SIGNAL(finished()), this, SLOT(showHideFinished()), Qt::UniqueConnection);
}

void MImWordTracker::fade(int frame)
{
    int showHideFrames = showHideTimeline.endFrame();
    showHideFrames = (showHideFrames > 0) ? showHideFrames : DefaultShowHideFrames;
    const qreal opacity = qreal(frame) / showHideFrames;
    parentWidget()->setOpacity(opacity);
    parentWidget()->update();
}


void MImWordTracker::showHideFinished()
{
    const bool hiding = (showHideTimeline.direction() == QTimeLine::Backward);

    if (hiding) {
        containerWidget->hide();
        emit regionChanged();
    }
}

void MImWordTracker::appear(bool withAnimation)
{
    if (!isVisible()) {
        if (withAnimation) {
            showHideTimeline.setDirection(QTimeLine::Forward);
            if (showHideTimeline.state() != QTimeLine::Running) {
                showHideTimeline.start();
            }
        }
        containerWidget->show();
        show();
    } else {
        containerWidget->update();
    }
    emit regionChanged();
}

void MImWordTracker::disappear(bool withAnimation)
{
    if (!isVisible())
        return;

    if (withAnimation) {
        showHideTimeline.setDirection(QTimeLine::Backward);
        if (showHideTimeline.state() != QTimeLine::Running) {
            showHideTimeline.start();
        }
        // will hide and emit regionChanged when timeline is finished
    } else {
        containerWidget->hide();
        emit regionChanged();
    }
}

void MImWordTracker::setPosition(const QPoint &pos)
{
    QRectF widgetRect, containerRect;
    QSizeF containerSize = preferredSize();
    containerSize.setHeight(containerSize.height() + pointerHeight());
    containerRect = QRectF(pos, containerSize);
    widgetRect = QRectF(QPointF(0, pointerHeight()), preferredSize());

    containerWidget->setGeometry(containerRect);
    setGeometry(widgetRect);

    if (isVisible()) {
        containerWidget->update();
        emit regionChanged();
    }
}

QRegion MImWordTracker::region() const
{
    QRegion ret;
    if (isVisible()) {
        ret = mapToScreenRect(containerWidget->geometry().toRect());
    }
    return ret;
}

void MImWordTracker::onThemeChangeCompleted()
{
    // reset time line
    setupTimeLine();
}

void MImWordTracker::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view)
{
    if (!isVisible())
        return;

    // Clear all with inactive color.
    reactionMap->setTransform(this, view);
    reactionMap->setInactiveDrawingValue();
    reactionMap->fillRectangle(geometry());

    // Draw the actual word tracker area.
    reactionMap->setReactiveDrawingValue();
    reactionMap->fillRectangle(geometry());
}
