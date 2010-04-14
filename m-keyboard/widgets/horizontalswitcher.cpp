/* * This file is part of m-keyboard *
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


#include "horizontalswitcher.h"
#include <QGraphicsSceneResizeEvent>
#include <QGraphicsScene>
#include <QDebug>

namespace
{
    const int SwitchDuration = 500;
    const int SwitchFrames = 300;
}

HorizontalSwitcher::HorizontalSwitcher(QGraphicsItem *parent) :
    QGraphicsWidget(parent),
    currentIndex(-1),
    animTimeLine(SwitchDuration),
    loopingEnabled(false)
{
    setObjectName("HorizontalSwitcher");

    animTimeLine.setFrameRange(0, SwitchFrames);

    enterAnim.setTimeLine(&animTimeLine);
    leaveAnim.setTimeLine(&animTimeLine);

    connect(&animTimeLine, SIGNAL(finished()), this, SLOT(finishAnimation()));
}

HorizontalSwitcher::~HorizontalSwitcher()
{
    if (isRunning())
        finishAnimation();

    // Delete all widgets that were not removed with removeWidget().
    qDeleteAll(slides);
    slides.clear();
}

void HorizontalSwitcher::switchTo(Direction direction)
{
    if (isRunning())
        finishAnimation();

    if (slides.count() <= 1)
        return;

    int newIndex;
    if (direction == Right) {
        if (!loopingEnabled && currentIndex == slides.count() - 1)
            return;
        newIndex = (currentIndex + 1) % slides.count();
    } else { // Left
        if (!loopingEnabled && currentIndex == 0)
            return;
        newIndex = currentIndex - 1;
        if (newIndex < 0)
            newIndex += slides.count();
    }

    QGraphicsWidget *currentWidget = slides.at(currentIndex);
    QGraphicsWidget *nextWidget = slides.at(newIndex);

    // Current item is about to leave
    leaveAnim.setItem(currentWidget);

    // New item is about to enter
    enterAnim.setItem(nextWidget);

    // Try to fit current size.
    nextWidget->resize(size());

    if (direction == Right) {
        // Set the new item to the right of this.
        nextWidget->setPos(this->size().width(), 0.0);
        enterAnim.setPosAt(0.0, nextWidget->pos());
        enterAnim.setPosAt(1.0, QPointF(0.0, 0.0));
        leaveAnim.setPosAt(0.0, currentWidget->pos());
        leaveAnim.setPosAt(1.0, QPointF(-currentWidget->size().width(), 0.0));
    } else {
        // Set the new item to the left of this.
        nextWidget->setPos(-nextWidget->size().width(), 0.0);
        enterAnim.setPosAt(0.0, nextWidget->pos());
        enterAnim.setPosAt(1.0, QPointF(0.0, 0.0));
        leaveAnim.setPosAt(0.0, currentWidget->pos());
        leaveAnim.setPosAt(1.0, QPointF(this->size().width(), 0.0));
    }

    nextWidget->show();

    emit switchStarting(currentIndex, newIndex);
    emit switchStarting(currentWidget, nextWidget);

    currentIndex = newIndex;
    animTimeLine.start();
}

void HorizontalSwitcher::switchTo(int index)
{
    if (slides.count() > 1) {
        if (!isVisible()) {
            setCurrent(index);
        } else if (index == currentIndex - 1)
            switchTo(Left);
        else if (index == currentIndex + 1)
            switchTo(Right);
        else if (index != currentIndex) {
            // TODO: support animated switching over multiple slides?
            setCurrent(index);
        }
    } else {
        setCurrent(index);
    }
}

void HorizontalSwitcher::switchTo(QGraphicsWidget *widget)
{
    Q_ASSERT(widget && slides.contains(widget));
    switchTo(slides.indexOf(widget));
}

void HorizontalSwitcher::setCurrent(QGraphicsWidget *widget)
{
    Q_ASSERT(widget && slides.contains(widget));
    setCurrent(slides.indexOf(widget));
}

void HorizontalSwitcher::setCurrent(int index)
{
    if (index >= 0 && index < slides.count() && index != currentIndex) {
        int oldIndex = -1;
        QGraphicsWidget *old = NULL;
        if (currentIndex >= 0 && currentIndex < slides.count()) {
            oldIndex = currentIndex;
            old = slides.at(currentIndex);
        }
        currentIndex = index;
        QGraphicsWidget *widget = slides.at(index);
        widget->setPos(0, 0);
        widget->resize(size());
        widget->show();
        emit switchStarting(oldIndex, currentIndex);
        emit switchStarting(old, widget);
        emit switchDone(oldIndex, currentIndex);
        emit switchDone(old, widget);

        updateGeometry();

        if (old) {
            old->hide();
        }
    }
}

int HorizontalSwitcher::current() const
{
    return (slides.isEmpty() ? -1 : currentIndex);
}

QGraphicsWidget *HorizontalSwitcher::currentWidget() const
{
    QGraphicsWidget *widget = NULL;
    if (current() >= 0) {
        widget = slides.at(currentIndex);
    }
    return widget;
}

QGraphicsWidget *HorizontalSwitcher::widget(int index)
{
    QGraphicsWidget *widget = NULL;
    if (index >= 0 && index < slides.count()) {
        widget = slides.at(index);
    }
    return widget;
}

int HorizontalSwitcher::count() const
{
    return slides.count();
}

bool HorizontalSwitcher::isRunning() const
{
    return (animTimeLine.state() == QTimeLine::Running);
}

void HorizontalSwitcher::setLooping(bool enable)
{
    loopingEnabled = enable;
}

void HorizontalSwitcher::setDuration(int ms)
{
    animTimeLine.setDuration(ms);
    animTimeLine.setFrameRange(0, ms * SwitchFrames / SwitchDuration);
}

void HorizontalSwitcher::addWidget(QGraphicsWidget *widget)
{
    if (widget) {
        widget->setParentItem(this);
        widget->setPreferredWidth(size().width());

        slides.append(widget);
        widget->hide();
    }
}

void HorizontalSwitcher::removeWidget(QGraphicsWidget *widget)
{
    if (widget && slides.contains(widget)) {
        if (widget->isVisible() && isRunning())
            finishAnimation();

        widget->setParentItem(0);
        if (widget->scene())
            widget->scene()->removeItem(widget);
        slides.removeOne(widget);

        if (!slides.isEmpty()) {
            if (--currentIndex < 0)
                currentIndex += slides.count();
            setCurrent(currentIndex);
        }
    }
}

void HorizontalSwitcher::removeAll()
{
    foreach(QGraphicsWidget * slide, slides) {
        slide->setParentItem(0);
        if (slide->scene())
            slide->scene()->removeItem(slide);
    }
    slides.clear();
    updateGeometry();
}


void HorizontalSwitcher::deleteAll()
{
    qDeleteAll(slides);
    slides.clear();
    currentIndex = -1;
    updateGeometry();
}


void HorizontalSwitcher::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsWidget *widget = currentWidget();
    if (widget) {
        widget->resize(event->newSize());
    }
}


QSizeF HorizontalSwitcher::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    // return the size hint of the currently visible widget
    QGraphicsWidget *widget = currentWidget();
    QSizeF hint;

    if (widget) {
        hint = widget->effectiveSizeHint(which, constraint);
    } else {
        hint = QGraphicsWidget::sizeHint(which, constraint);
    }

    return hint;
}


void HorizontalSwitcher::finishAnimation()
{
    int oldIndex = -1;

    // Hide old item
    QGraphicsWidget *old = static_cast<QGraphicsWidget *>(leaveAnim.item());
    if (old) {
        oldIndex = slides.indexOf(old);
        old->hide();
    }

    // Clear transformations
    leaveAnim.clear();
    enterAnim.clear();

    animTimeLine.stop();

    // Discard cached sizehint info before telling that the switch is done.
    updateGeometry();

    emit switchDone(oldIndex, currentIndex);
    emit switchDone(old, slides.at(currentIndex));
}
