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
#include "enginemanager.h"

#include <mimenginetypes.h>
#include <mimenginefactory.h>
#include <QGraphicsWidget>
#include <QDebug>

MImCorrectionHost::MImCorrectionHost(MWidget *window, QObject *parent)
    : AbstractEngineWidgetHost(window, parent),
      ReactionMapPaintable(),
      rotationInProgress(false),
      currentMode(AbstractEngineWidgetHost::FloatingMode),
      pendingCandidatesUpdate(false),
      wordTracker(new MImWordTracker(window)),
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

QGraphicsWidget *MImCorrectionHost::engineWidget() const
{
    QGraphicsWidget *widget = 0;

    if (isActive()) {
        widget = wordTracker->isVisible() ? qobject_cast<QGraphicsWidget *>(wordTracker)
                 : qobject_cast<QGraphicsWidget *>(wordList);
    }
    return widget;
}

QGraphicsWidget *MImCorrectionHost::inlineWidget() const
{
    return qobject_cast<QGraphicsWidget *>(wordTracker);
}

bool MImCorrectionHost::isActive() const
{
    return (wordTracker->isVisible() || wordList->isVisible());
}

bool MImCorrectionHost::typedWordIsInDictionary()
{
    if (!EngineManager::instance().engine()) {
        return false;
    }

    return EngineManager::instance().engine()->candidateSource(0) != MImEngine::DictionaryTypeInvalid;
}

void MImCorrectionHost::setCandidates(const QStringList &list)
{
    mCandidates = list;
    suggestionString.clear();
    if (mCandidates.isEmpty()) {
        return;
    }

    // The first candidate is always the original input word.
    // So if there are more than one suggestions, the second one is
    // the suggestion word.
    suggestionString = mCandidates.at(0);
    if (mCandidates.count() > 1) {
        suggestionString = mCandidates.at(1);
    }

    if (isActive()) {
        if (currentMode == AbstractEngineWidgetHost::FloatingMode) {
            wordTracker->setCandidate(suggestionString);
        } else {
            wordList->setCandidates(mCandidates, typedWordIsInDictionary());
        }
    } else {
        pendingCandidatesUpdate = true;
    }
}

void MImCorrectionHost::appendCandidates(int startPos, const QStringList &candidateList)
{
    Q_UNUSED(startPos);
    Q_UNUSED(candidateList);
}

QStringList MImCorrectionHost::candidates() const
{
    return mCandidates;
}

void MImCorrectionHost::showEngineWidget(AbstractEngineWidgetHost::DisplayMode mode)
{
    bool modeChanged = (currentMode != mode);
    currentMode = mode;

    if (mCandidates.isEmpty()) {
        hideEngineWidget();
        return;
    }
    if (pendingCandidatesUpdate || modeChanged) {
        if (currentMode == AbstractEngineWidgetHost::FloatingMode) {
            wordTracker->setCandidate(suggestionString);
        } else {
            wordList->setCandidates(mCandidates, typedWordIsInDictionary());
        }
        pendingCandidatesUpdate = false;
    }

    if (currentMode == AbstractEngineWidgetHost::FloatingMode) {
        suggestionString = wordTracker->candidate();

        if (!wordTracker->isVisible()) {
            wordList->disappear();
            wordTracker->appear(false);
        }
    } else {
        wordTracker->disappear(false);
        wordList->appear();
    }
}

void MImCorrectionHost::hideEngineWidget()
{
    wordTracker->disappear(false);
    wordList->disappear();
}

AbstractEngineWidgetHost::DisplayMode MImCorrectionHost::displayMode() const
{
    return currentMode;
}

void MImCorrectionHost::watchOnWidget(QGraphicsWidget *widget)
{
    Q_UNUSED(widget);
}

void MImCorrectionHost::setPosition(const QRect &cursorRect)
{
    if (cursorRect.isNull() || !cursorRect.isValid()) {
        return;
    }

    wordTracker->setPosition(cursorRect);
}

void MImCorrectionHost::handleNavigationKey(AbstractEngineWidgetHost::NaviKey key)
{
    Q_UNUSED(key);
}

int MImCorrectionHost::suggestedWordIndex() const
{
    return mCandidates.indexOf(suggestionString);
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
        showEngineWidget(currentMode);
        rotationInProgress = false;
    }
}

void MImCorrectionHost::reset()
{
    setCandidates(QStringList());
    hideEngineWidget();
}

void MImCorrectionHost::setPageIndex(int index)
{
    Q_UNUSED(index);
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
        emit candidateClicked(candidate, mCandidates.indexOf(suggestionString));
    }

    hideEngineWidget();
}

bool MImCorrectionHost::isPaintable() const
{
    return isActive();
}

bool MImCorrectionHost::isFullScreen() const
{
    // Correction candidate widget occupies whole screen when it is AbstractEngineWidgetHost::DialogMode.
    return displayMode() == AbstractEngineWidgetHost::DialogMode;
}

void MImCorrectionHost::longTap()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!isActive())
        return;

    showEngineWidget(AbstractEngineWidgetHost::DialogMode);
}

void MImCorrectionHost::setTitle(QString &title)
{
    Q_UNUSED(title);
}
