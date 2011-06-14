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
      wordTrackerContainer(new QGraphicsWidget(window)),
      wordTracker(new MImWordTracker(wordTrackerContainer)),
      wordTrackerHiddenByRotation(false),
      wordList(new MImWordList())
{
    connect(wordTracker, SIGNAL(candidateClicked(QString)), this, SLOT(handleCandidateClicked(QString)));
    connect(wordTracker, SIGNAL(longTapped()), this, SLOT(longTap()));
    // The word tracker changed -> Repaint the reaction maps
    // TODO: The reaction map repainting can be optimized here to clear/repaint only the
    // reaction map of the word tracker.
    connect(wordTracker, SIGNAL(makeReactionMapDirty()), &signalForwarder, SIGNAL(requestRepaint()));

    connect(wordList, SIGNAL(candidateClicked(QString)), this, SLOT(handleCandidateClicked(QString)));
    // The word list goes on top -> Clear the reaction maps
    connect(wordList, SIGNAL(displayEntered()), &signalForwarder, SIGNAL(requestClear()));
    // The word list disappears -> Repaint the reaction maps
    connect(wordList, SIGNAL(displayExited()), &signalForwarder, SIGNAL(requestRepaint()));
}


MImCorrectionHost::~MImCorrectionHost()
{
    if (!wordTrackerContainer.isNull())
        delete wordTrackerContainer;
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
    return (wordTracker->isVisible() || wordList->isVisible() || wordTrackerHiddenByRotation);
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

void MImCorrectionHost::appendCandidates(const QStringList &candidateList)
{
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
    // if the wordTracker is really temporary hidden, caller must set a flag
    wordTrackerHiddenByRotation = false;
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
        if (displayMode() == AbstractEngineWidgetHost::FloatingMode) {
            hideEngineWidget();
            wordTrackerHiddenByRotation = true;
        }
    }
}

void MImCorrectionHost::finalizeOrientationChange()
{
}

void MImCorrectionHost::handleAppOrientationChanged()
{
    if (rotationInProgress) {
        if (isActive() && displayMode() == AbstractEngineWidgetHost::FloatingMode) {
            showEngineWidget(currentMode);
            wordTrackerHiddenByRotation = false;
        }
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
