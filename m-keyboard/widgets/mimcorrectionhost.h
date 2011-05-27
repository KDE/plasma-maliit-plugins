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

#ifndef MIMCORRECTIONHOST_H
#define MIMCORRECTIONHOST_H

#include "abstractenginewidgethost.h"
#include "reactionmappaintable.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QRegion>
#include <QPointer>

class QRect;
class MSceneWindow;
class MReactionMap;
class MImWordTracker;
class MImWordList;
class QGraphicsView;
class MImEngineWordsInterface;

/*!
  \class MImCorrectionHost
  \brief The MImCorrectionHost class is used to show error correction
  candidate word tracker or word list.
*/
class MImCorrectionHost : public AbstractEngineWidgetHost, public ReactionMapPaintable
{
    Q_OBJECT

    friend class Ut_MImCorrectionHost;

public:
    /*! Constructor
     *
     */
    explicit MImCorrectionHost(MWidget *window, QObject *parent = 0);

    /*! Destructor
     *
     */
    ~MImCorrectionHost();

    /*!
     * \brief Returns true if candidate zero, the originally typed word, is in the dictionary.
     */
    bool typedWordIsInDictionary();

    //! reimp
    virtual QGraphicsWidget *engineWidget() const;
    virtual QGraphicsWidget *inlineWidget() const;
    virtual bool isActive() const;
    virtual void setTitle(QString &title);
    virtual void setCandidates(const QStringList &candidates);
    virtual void appendCandidates(const QStringList &candidate);
    virtual QStringList candidates() const;
    virtual void showEngineWidget(DisplayMode mode = FloatingMode);
    virtual void hideEngineWidget();
    virtual DisplayMode displayMode() const;
    virtual void watchOnWidget(QGraphicsWidget *widget);
    virtual void setPosition(const QRect &cursorRect);
    virtual void handleNavigationKey(NaviKey key);
    virtual int suggestedWordIndex() const;
    virtual void prepareToOrientationChange();
    virtual void finalizeOrientationChange();
    virtual void reset();
    virtual void setPageIndex(int index = 0);
    virtual void handleAppOrientationChanged();
    //! reimp_end

    /*!
     * \brief Draw its reactive areas onto the reaction map
     */
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

    /*! \reimp */
    bool isPaintable() const;
    bool isFullScreen() const;
    /*! \reimp_end */

protected slots:
    void handleCandidateClicked(const QString &candidate);

    void longTap();

private:
    bool rotationInProgress;
    QStringList mCandidates;
    DisplayMode currentMode;
    QString suggestionString;
    bool pendingCandidatesUpdate;

    QPointer<QGraphicsWidget> wordTrackerContainer;
    MImWordTracker *wordTracker;
    bool wordTrackerHiddenByPosition;
    bool wordTrackerHiddenByRotation;
    MImWordList *wordList;
    MImEngineWordsInterface *correctionEngine;

    Q_DISABLE_COPY(MImCorrectionHost)
};

#endif
