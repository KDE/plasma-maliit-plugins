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
#include "mimwordlist.h"
#include "mimwordlistitem.h"
#include <mplainwindow.h>

#include <QGraphicsLinearLayout>
#include <QDebug>
#include <QString>

#include <MSceneManager>
#include <MScene>
#include <MGConfItem>
#include <MContentItem>
#include <MTheme>
#include <mreactionmap.h>

#include <mwidgetcreator.h>
M_REGISTER_WIDGET_NO_CREATE(MImWordList)

namespace
{
    const char * const WordListObjectName = "CorrectionWordList";
};

MIMWordListWindow::MIMWordListWindow(MImWordList *widget)
    : MImOverlay(),
      listWidget(widget)
{
    setVisible(false);
}

bool MIMWordListWindow::sceneEvent(QEvent *e)
{
    // TODO: below hiding list widget could be removed when meegotouch
    // decide not hiding dialog when tap outside.
    if (e->type() == QEvent::GraphicsSceneMouseRelease) {
        listWidget->disappear();
    }
    return MImOverlay::sceneEvent(e);
}

void MIMWordListWindow::handleListAppeared()
{
    setVisible(true);
    listWidget->setParentItem(this);
}

void MIMWordListWindow::handleListDisappeared()
{
    setVisible(false);
}

MImWordList::MImWordList()
    : MDialog(),
      parentWindow(new MIMWordListWindow(this))
{
   RegionTracker::instance().addRegion(*this);
    // for MATTI
    setObjectName(WordListObjectName);

    MWidget *contentWidget = new MWidget(this);
    mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    contentWidget->setLayout(mainLayout);

    for (int i = 0; i < MaxCandidateCount; i++) {
        candidateItems[i] = new MImWordListItem(contentWidget);
        candidateItems[i]->setVisible(false);
        connect(candidateItems[i], SIGNAL(clicked()), this, SLOT(select()));
        mainLayout->addItem(candidateItems[i]);
    }
    setCentralWidget(contentWidget);
    hide();

    connect(this, SIGNAL(appeared()),
            parentWindow, SLOT(handleListAppeared()));
    connect(this, SIGNAL(disappeared()),
            parentWindow, SLOT(handleListDisappeared()));
}

MImWordList::~MImWordList()
{
    setParentItem(0);
    delete parentWindow;
    parentWindow = 0;
}

void MImWordList::setCandidates(const QStringList &candidates)
{
    mCandidates = candidates.mid(0, MaxCandidateCount);

    for (int i = 0; i < MaxCandidateCount; i++) {
        if (i < candidates.count()) {
            candidateItems[i]->setSelected(false);
            mainLayout->addItem(candidateItems[i]);
            candidateItems[i]->setTitle(candidates.at(i));
            candidateItems[i]->setVisible(true);
            candidateItems[i]->setEnabled(true);
            candidateItems[i]->setLayoutPosition(M::VerticalCenterPosition);
        } else {
            mainLayout->removeItem(candidateItems[i]);
            candidateItems[i]->setVisible(false);
        }
    }
    mainLayout->invalidate();
}

QStringList MImWordList::candidates() const
{
    return mCandidates;
}

void MImWordList::setHighlightCandidate(const QString &candidate)
{
    int index = mCandidates.indexOf(candidate);
    if (index >= 0) {
        candidateItems[index]->setSelected(true);
    }
}

void MImWordList::select()
{
    // ignore the select actions during animation
    if (!isVisible() || sceneWindowState() == MSceneWindow::Appearing
        || sceneWindowState() == MSceneWindow::Disappearing) {
        return;
    }
    MImWordListItem *item = qobject_cast<MImWordListItem *> (sender());
    if (item) {
        const QString candidate = item->title();
        emit candidateClicked(candidate);
    }
}

void MImWordList::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *)
{
    if (!isVisible())
        return;
    // word list take whole screen. And inner contentitem will play their
    // default feedback. Don't need reaction map.
    reactionMap->setInactiveDrawingValue();
    reactionMap->setTransform(QTransform());
    reactionMap->fillRectangle(0, 0, reactionMap->width(), reactionMap->height());
}
