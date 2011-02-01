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

#include <QDebug>

MImCorrectionHost::MImCorrectionHost(MSceneWindow *parentWindow)
    : QObject(parentWindow),
      ReactionMapPaintable(),
      rotationInProgress(false),
      currentMode(MImCorrectionHost::WordTrackerMode),
      pendingCandidatesUpdate(false),
      wordTracker(new MImWordTracker(parentWindow)),
      wordList(new MImWordList())
{
    connect(wordTracker, SIGNAL(candidateClicked(QString)), this, SLOT(handleCandidateClicked(QString)));
    connect(wordTracker, SIGNAL(longTapped()), this, SLOT(longTap()));
    // The word tracker changed -> Repaint the reaction maps
    // TODO: The reaction map repainting can be optimized here to clear/repaint only the
    // reaction map of the word tracker.
    connect(wordTracker, SIGNAL(geometryChanged()), &signalForwarder, SIGNAL(requestRepaint()));
    connect(wordTracker, SIGNAL(displayExited()), &signalForwarder, SIGNAL(requestRepaint()));

    connect(wordList, SIGNAL(candidateClicked(QString)), this, SLOT(handleCandidateClicked(QString)));
    // The word list goes on top -> Clear the reaction maps
    connect(wordList, SIGNAL(displayEntered()), &signalForwarder, SIGNAL(requestClear()));
    // The word list disappears -> Repaint the reaction maps
    connect(wordList, SIGNAL(displayExited()), &signalForwarder, SIGNAL(requestRepaint()));
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

    if (isActive()) {
        if (currentMode == WordTrackerMode) {
            wordTracker->setCandidate(suggestionString);
        } else {
            wordList->setCandidates(candidates);
        }
    } else {
        pendingCandidatesUpdate = true;
    }
}

void MImCorrectionHost::setPosition(const QRect &cursorRect)
{
    if (cursorRect.isNull() || !cursorRect.isValid()) {
        return;
    }

    wordTracker->setPosition(cursorRect);
}

void MImCorrectionHost::showCorrectionWidget(MImCorrectionHost::CandidateMode mode)
{
    bool modeChanged = (currentMode != mode);
    currentMode = mode;

    if (candidates.isEmpty()) {
        hideCorrectionWidget();
        return;
    }
    if (pendingCandidatesUpdate || modeChanged) {
        if (currentMode == WordTrackerMode) {
            wordTracker->setCandidate(suggestionString);
        } else {
            wordList->setCandidates(candidates);
        }
        pendingCandidatesUpdate = false;
    }

    if (currentMode == WordTrackerMode) {
        suggestionString = wordTracker->candidate();

        if (!wordTracker->isVisible()) {
            wordList->disappear();
            wordTracker->appear(false);
        }
    } else {
        // Always highlight the first item in the candidate list
        // which is the origin input word
        wordList->setHighlightCandidate(candidates.at(0));
        wordTracker->disappear(false);
        wordList->appear();
    }
}

void MImCorrectionHost::hideCorrectionWidget()
{
    wordTracker->disappear(false);
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
    }
}

void MImCorrectionHost::finalizeOrientationChange()
{
    if (rotationInProgress) {
        showCorrectionWidget(currentMode);
        rotationInProgress = false;
    }
}

void MImCorrectionHost::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view)
{
    if (!isActive())
        return;

    if (wordTracker->isVisible()) {
        wordTracker->paintReactionMap(reactionMap, view);
    }
    if (wordList->isVisible()) {
        wordList->paintReactionMap(reactionMap, view);
    }
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

bool MImCorrectionHost::isPaintable() const
{
    return isActive();
}

bool MImCorrectionHost::isFullScreen() const
{
    // Correction candidate widget occupies whole screen when it is WordListMode.
    return candidateMode() == MImCorrectionHost::WordListMode;
}

void MImCorrectionHost::longTap()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!isActive())
        return;

    showCorrectionWidget(WordListMode);
}
