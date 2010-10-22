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


#include "mimcorrectionhost.h"
#include "mimwordtracker.h"
#include "mimwordlist.h"

#include <QGraphicsLinearLayout>
#include <QDebug>
#include <QString>

#include <MSceneWindow>
#include <MSceneManager>
#include <mreactionmap.h>
#include <mplainwindow.h>
#include <MGConfItem>

namespace
{
    const int CandidatesPreeditGap = 10; // Gap between pre-edit and candidates
    const int MinimumCandidateWidget = 64; // Minimum length of a candidate.
};

MImCorrectionHost::MImCorrectionHost(MSceneWindow *parentWindow)
    : QObject(parentWindow),
      rotationInProgress(false),
      wordTrackerPosition(0, 0),
      currentMode(MImCorrectionHost::WordTrackerMode),
      wordTracker(new MImWordTracker(parentWindow)),
      wordList(new MImWordList())
{
    connect(wordTracker, SIGNAL(candidateClicked(QString)), this, SLOT(handleCandidateClicked(QString)));
    connect(wordTracker, SIGNAL(regionChanged()), this, SLOT(sendRegion()));

    connect(wordList, SIGNAL(candidateClicked(QString)), this, SLOT(handleCandidateClicked(QString)));
    connect(wordList, SIGNAL(regionChanged()), this, SLOT(sendRegion()));
}


MImCorrectionHost::~MImCorrectionHost()
{
    delete wordTracker;
    delete wordList;
}

bool MImCorrectionHost::isActive() const
{
    return (wordTracker->isVisible() || wordList->isVisible());
}

void MImCorrectionHost::setCandidates(const QStringList list)
{
    candidates = list;
    suggestionString.clear();
    if (candidates.isEmpty()) {
        return;
    }
    // The first candidate is always the original input word.
    // So if there are more than one suggestions, the second one is
    // the suggestion word.
    suggestionString = candidates.at(0);
    if (candidates.count() > 1) {
        suggestionString = candidates.at(1);
    }

    wordTracker->setCandidate(suggestionString);
    wordList->setCandidates(candidates);
}

QPoint MImCorrectionHost::position() const
{
    return wordTrackerPosition;
}

void MImCorrectionHost::setPosition(const QPoint &position)
{
    wordTrackerPosition = position;

    int sceneWidth = MPlainWindow::instance()->sceneManager()->visibleSceneSize().width();
    wordTrackerPosition.setX(qBound(0, wordTrackerPosition.x(), (int)(sceneWidth - wordTracker->idealWidth())));

    wordTracker->setPosition(wordTrackerPosition);
}

void MImCorrectionHost::setPosition(const QRect &preeditRect)
{
    if (preeditRect.isNull() || !preeditRect.isValid()) {
        setPosition(QPoint(0, 0));
        return;
    }

    QPoint position;

    // Set horizontal position
    // the right side correction widget is aligned with the right side of
    // pre-edit rectangle + MinimumCandidateWidget
    position.setX(preeditRect.right() + MinimumCandidateWidget - wordTracker->idealWidth());

    // Set vertical position
    // Vertically the candidatesWidget is below the pre-edit + CandidatesPreeditGap.
    position.setY(preeditRect.bottom() + CandidatesPreeditGap);
    setPosition(position);
}

void MImCorrectionHost::showCorrectionWidget(MImCorrectionHost::CandidateMode mode)
{
    currentMode = mode;

    if (candidates.isEmpty()) {
        hideCorrectionWidget(false);
        return;
    }

    if (currentMode == WordTrackerMode) {
        suggestionString = wordTracker->candidate();

        if (!wordTracker->isVisible()) {
            wordList->disappear();
            wordTracker->appear();
        }
    } else {
        // Always highlight the first item in the candidate list
        // which is the origin input word
        wordList->setHighlightCandidate(candidates.at(0));
        wordTracker->disappear();
        wordList->appear();
    }
}

void MImCorrectionHost::hideCorrectionWidget(bool withAnimation)
{
    wordTracker->disappear(withAnimation);
    wordList->disappear();
}

MImCorrectionHost::CandidateMode MImCorrectionHost::candidateMode() const
{
    return currentMode;
}

QString MImCorrectionHost::suggestion() const
{
    return suggestionString;
}

void MImCorrectionHost::prepareToOrientationChange()
{
    if (isActive()) {
        rotationInProgress = true;
        hideCorrectionWidget();
    }
}

void MImCorrectionHost::finalizeOrientationChange()
{
    if (rotationInProgress) {
        showCorrectionWidget(currentMode);
        rotationInProgress = false;
    }
}

void MImCorrectionHost::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *)
{
    // Clear all with inactive color.
    reactionMap->setInactiveDrawingValue();
    reactionMap->setTransform(QTransform());
    reactionMap->fillRectangle(0, 0, reactionMap->width(), reactionMap->height());

    // Draw the actual candidate candidatesWidget area.
    reactionMap->setReactiveDrawingValue();

    if (wordTracker->isVisible()) {
        reactionMap->fillRectangle(wordTracker->geometry());
    }
    if (wordList->isVisible()) {
        reactionMap->fillRectangle(wordList->geometry());
    }
}

void MImCorrectionHost::sendRegion()
{
    QRegion region;
    if (isActive()) {
        if (currentMode == WordListMode) {
            region = wordList->region();
        } else {
            region = wordTracker->region();
        }
    }
    emit regionUpdated(region);
}

void MImCorrectionHost::handleCandidateClicked(const QString &candidate)
{
    if (!candidate.isEmpty() && isActive()) {
        suggestionString = candidate;
        emit candidateClicked(candidate);
    }

    hideCorrectionWidget();
}

void MImCorrectionHost::reset()
{
    setCandidates(QStringList());
    hideCorrectionWidget();
}
