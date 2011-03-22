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
#include "reactionmapwrapper.h"

#include <QGraphicsLinearLayout>
#include <QDebug>
#include <QString>

#include <mwidgetcreator.h>
#include <MSeparator>
M_REGISTER_WIDGET_NO_CREATE(MImWordList)

namespace
{
    const char * const WordListObjectName = "CorrectionWordList";

    void addItem(QGraphicsLinearLayout *layout, MWidgetController *item, int position)
    {
        item->setSelected(false);
        layout->insertItem(position, item);
        item->setVisible(true);
        item->setEnabled(true);
        item->setLayoutPosition(M::VerticalCenterPosition);
    }

    void removeItem(QGraphicsLinearLayout *layout, MWidgetController *item)
    {
        layout->removeItem(item);
        item->setVisible(false);
    }
};


MImWordList::MImWordList()
    : MDialog()
{
    RegionTracker::instance().addRegion(*this);
    // for MATTI
    setObjectName(WordListObjectName);

    MWidget *contentWidget = new MWidget(this);
    mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    contentWidget->setLayout(mainLayout);

    addToDictionaryItem = new MImWordListItem(contentWidget);
    //% "Add to dictionary"
    addToDictionaryItem->setTitle(qtTrId("qtn_vkb_dictionary_add"));
    addToDictionaryItem->setVisible(false);
    connect(addToDictionaryItem, SIGNAL(clicked()), this, SLOT(select()));
    mainLayout->addItem(addToDictionaryItem);

    addToDictionarySeparator = new MSeparator(contentWidget);
    addToDictionarySeparator->setStyleName("CommonItemDivider");
    addToDictionarySeparator->setVisible(false);
    mainLayout->addItem(addToDictionarySeparator);

    for (int i = 0; i < MaxCandidateCount; i++) {
        candidateItems[i] = new MImWordListItem(contentWidget);
        candidateItems[i]->setVisible(false);
        connect(candidateItems[i], SIGNAL(clicked()), this, SLOT(select()));
        mainLayout->addItem(candidateItems[i]);
    }
    setCentralWidget(contentWidget);
    hide();
}

MImWordList::~MImWordList()
{
}

void MImWordList::setCandidates(const QStringList &candidates, bool typedWordIsInDictionary)
{
    if (candidates.isEmpty()) {
        qWarning() << __PRETTY_FUNCTION__ << "empty candidates list";
    } else {
        // First candidate, the originally typed word, is show in the dialog header
        setTitle(candidates[0]);
    }

    int candidateItemsStartIndex = 0;
    if (typedWordIsInDictionary) {
        mCandidates = candidates.mid(1, MaxCandidateCount);

        removeItem(mainLayout, addToDictionaryItem);
        removeItem(mainLayout, addToDictionarySeparator);
    } else {
        mCandidates = candidates.mid(0, MaxCandidateCount);
        candidateItemsStartIndex = 1;

        addItem(mainLayout, addToDictionaryItem, 0);
        addItem(mainLayout, addToDictionarySeparator, 1);
    }

    for (int i = 0; i < MaxCandidateCount; i++) {
        if (i < mCandidates.count() && i >= candidateItemsStartIndex) {
            candidateItems[i]->setTitle(mCandidates.at(i));
            addItem(mainLayout, candidateItems[i], -1);
        } else {
            removeItem(mainLayout, candidateItems[i]);
        }
    }
    mainLayout->invalidate();
}

QStringList MImWordList::candidates() const
{
    return mCandidates;
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
        const QString candidate = (item == addToDictionaryItem) ? title() : item->title();
        emit candidateClicked(candidate);
    }
}

void MImWordList::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *)
{
#ifndef HAVE_REACTIONMAP
    Q_UNUSED(reactionMap);
    return;
#else
    if (!isVisible())
        return;
    // word list take whole screen. And inner contentitem will play their
    // default feedback. Don't need reaction map.
    reactionMap->setInactiveDrawingValue();
    reactionMap->setTransform(QTransform());
    reactionMap->fillRectangle(0, 0, reactionMap->width(), reactionMap->height());
#endif // HAVE_REACTIONMAP
}
