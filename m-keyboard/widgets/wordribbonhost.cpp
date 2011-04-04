/* * This file is part of meego-keyboard-zh *
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

#include "wordribbonhost.h"
#include "wordribbon.h"
#include "wordribbondialog.h"
#include "regiontracker.h"
#include "enginemanager.h"

#include <mimenginewordsinterface.h>
#include <mreactionmap.h>
#include <MSceneManager>
#include <mplainwindow.h>
#include <MStylableWidget>

#include <limits>
#include <QtGui>
#include <QtCore>

namespace {
    const int MaximumWordRibbonDialogCount = 500;
    const int InitialCandidateCount = 20;
}

WordRibbonHost::WordRibbonHost(MWidget *w, QObject *parent):
        AbstractEngineWidgetHost(w, parent),
        ReactionMapPaintable(),
        wordRibbon(new WordRibbon(WordRibbon::RibbonStyleMode, w)),
        ribbonDialog(new WordRibbonDialog()),
        uniqueIndex(-1),
        candidatesCache(MaximumWordRibbonDialogCount)
{
    setObjectName("WordRibbonHost");

    bool ok = false;
    ribbonDialog->setVisible(false);
    connect(ribbonDialog, SIGNAL(finished(int)), this, SLOT(handleDialogClosed()));
    connect(ribbonDialog, SIGNAL(candidateClicked(QString,int)), this, SIGNAL(candidateClicked(QString,int)));
    // The dialog goes on top -> Clear the reaction maps
    connect(ribbonDialog, SIGNAL(displayEntered()), &signalForwarder, SIGNAL(requestClear()));
    // The dialog disappears -> Repaint the reaction maps
    connect(ribbonDialog, SIGNAL(displayExited()), &signalForwarder, SIGNAL(requestRepaint()));

    ok = connect(wordRibbon, SIGNAL(itemClicked(QString, int)), this, SIGNAL(candidateClicked(QString, int)));
    Q_ASSERT(ok);
    ok = connect(wordRibbon, SIGNAL(moreCandidatesRequested()), this, SLOT(openWordRibbonDialog()));
    Q_ASSERT(ok);
    wordRibbon->hide();

    RegionTracker::instance().addRegion(*wordRibbon);
    RegionTracker::instance().addInputMethodArea(*wordRibbon);
}

WordRibbonHost::~WordRibbonHost()
{
    if(ribbonDialog != NULL)
    {
        delete ribbonDialog;
        ribbonDialog = NULL;
    }
}

void WordRibbonHost::setTitle(QString &title)
{
    dialogTitle = title;
}

void WordRibbonHost::setCandidates(const QStringList &candidates)
{
    qDebug() <<Q_FUNC_INFO <<" candidates COUNT = " <<candidates.count();
    if (candidates.isEmpty()) {
        clearCandidate();
    }
    else {
        candidatesCache.clear();
        if (candidates.count() > candidatesCache.capacity()) {
            candidatesCache.setCapacity(candidates.count() );
        }
        for (int i = 0; i < candidates.count(); ++i) {
            candidatesCache.insert(i, candidates[i]);
        }
        wordRibbon->repopulate(candidates);
    }
}

void WordRibbonHost::watchOnWidget(QGraphicsWidget *widget)
{
    if (!widget) {
        return;
    }

    wordRibbon->setZValue(widget->zValue());
    connect(widget, SIGNAL(yChanged()), this, SLOT(updatePosition()));
    connect(widget, SIGNAL(visibleChanged()), this, SLOT(updatePosition()));
    watchedWidgets.append(widget);
    updatePosition();

    watchedWidgets.removeAll(QPointer<QGraphicsWidget>()); //remove all invalid pointers
}

void WordRibbonHost::prepareToOrientationChange()
{

}

void WordRibbonHost::finalizeOrientationChange()
{
    QStringList tmpList;
    for (int i = 0; i < InitialCandidateCount && i < candidatesCache.count(); ++i) {
        tmpList << this->candidatesCache.at(i);
    }
    wordRibbon->finalizeOrientationChange();
    wordRibbon->repopulate(tmpList);


    if (ribbonDialog->isVisible()){
        ribbonDialog->finalizeOrientationChange();
    }
}

void WordRibbonHost::showEngineWidget(DisplayMode mode)
{
    if (mode != AbstractEngineWidgetHost::DockedMode)
        return;

    if (candidatesCache.isEmpty()){
        hideEngineWidget();
        return;
    }

    wordRibbon->show();

    updatePosition();
}

void WordRibbonHost::hideEngineWidget()
{
    wordRibbon->hide();

    updatePosition();
}

void WordRibbonHost::clearCandidate()
{
    candidatesCache.clear();
    wordRibbon->removeAllItems();
    ribbonDialog->accept();
}

void WordRibbonHost::openWordRibbonDialog()
{
    //must fetch before show full dialog
    fetchMoreCandidates();

    QStringList list;
    ribbonDialog->appear(MPlainWindow::instance());
    for (int i = candidatesCache.firstIndex();
        i <= candidatesCache.lastIndex(); ++i) {
        list << candidatesCache.at(i);
    }
    ribbonDialog->setCandidates(list, dialogTitle);
}

void WordRibbonHost::handleDialogClosed()
{
}

void WordRibbonHost::updatePosition()
{
    qreal bottom(std::numeric_limits<qreal>::max());
    bool widgetVisible(false);

    foreach (const QGraphicsWidget *widget, watchedWidgets) {
        if (widget && widget->isVisible()) {
            bottom = qMin(widget->pos().y(), bottom);
            widgetVisible = true;
        }
    }

    const QPointF newPos(0, bottom - wordRibbon->size().height());
    if (widgetVisible) {
        wordRibbon->setPos(newPos);
    }
}

void WordRibbonHost::handleNavigationKey(NaviKey key)
{
    if (isEmpty())
        return;

    wordRibbon->handleNavigationKey(key);
}

AbstractEngineWidgetHost::DisplayMode WordRibbonHost::displayMode() const
{
    if (ribbonDialog->isVisible())
        return AbstractEngineWidgetHost::DialogMode;
    else
        return AbstractEngineWidgetHost::DockedMode;
}

void WordRibbonHost::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view)
{
    if (ribbonDialog->isVisible())
        ribbonDialog->paintReactionMap(reactionMap, view);
    else
        wordRibbon->paintReactionMap(reactionMap, view);
}

bool WordRibbonHost::isPaintable() const
{
    return isActive();
}

bool WordRibbonHost::isFullScreen() const
{
    return displayMode() == AbstractEngineWidgetHost::DialogMode;
}

void WordRibbonHost::appendCandidates(int startPos, const QStringList &candidate)
{
    if (startPos + candidate.count() > this->candidatesCache.capacity()) {
        this->candidatesCache.setCapacity(startPos + candidate.count());
    }
    for (int i = 0; i < candidate.count(); ++i) {
        this->candidatesCache.insert(startPos + i, candidate[i]);
    }
}

void WordRibbonHost::fetchMoreCandidates()
{
    // No need to fetch if it the cache is already full
    if ((candidatesCache.lastIndex() + 1) >= MaximumWordRibbonDialogCount)
        return;

    // fetch more candidates if there's room in the cache
    int requestLength = MaximumWordRibbonDialogCount - candidatesCache.lastIndex() - 1;
    int requestStartPos = candidatesCache.lastIndex() + 1;

    QStringList candidatesFromEngine;
    candidatesFromEngine = EngineManager::instance().engine()->candidates(requestStartPos, requestLength);
    appendCandidates(requestStartPos, candidatesFromEngine);
}

void WordRibbonHost::setPageIndex(int index)
{
    if (index != 0)
        return;

    if (candidatesCache.isEmpty() )
        return ;

    QStringList tmpList;
    for (int i = 0; i < InitialCandidateCount && i < candidatesCache.count(); ++i) {
        tmpList << this->candidatesCache.at(i);
    }
    wordRibbon->repopulate(tmpList);
}

bool WordRibbonHost::isActive() const
{
    return (wordRibbon->isVisible() || ribbonDialog->isVisible() );
}

QStringList WordRibbonHost::candidates() const
{
    QStringList retList;
    for(int i = 0; i < candidatesCache.count(); i++) {
        retList << candidatesCache.at(i);
    }
    return retList;
}

void WordRibbonHost::setPosition(const QRect &cursorRect)
{
    Q_UNUSED(cursorRect);
}

bool WordRibbonHost::isEmpty()
{
    return candidatesCache.isEmpty();
}

int WordRibbonHost::suggestedWordIndex() const
{
    return -1;
}

void WordRibbonHost::reset()
{
    clearCandidate();
}

QGraphicsWidget * WordRibbonHost::engineWidget() const
{
    QGraphicsWidget *widget = 0;

    if (isActive()) {
        widget = wordRibbon->isVisible() ? qobject_cast<QGraphicsWidget *>(wordRibbon)
                 : qobject_cast<QGraphicsWidget *>(ribbonDialog);
    }
    return widget;
}

QGraphicsWidget * WordRibbonHost::inlineWidget() const
{

    return qobject_cast<QGraphicsWidget *>(wordRibbon);
}
