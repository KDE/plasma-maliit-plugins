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

#ifndef MIMCORRECTIONHOST_H
#define MIMCORRECTIONHOST_H

#include "abstractenginewidgethost.h"
#include "reactionmappaintable.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QRegion>

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
    virtual void appendCandidates(int startPos, const QStringList &candidate);
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

    MImWordTracker *wordTracker;
    MImWordList *wordList;
    MImEngineWordsInterface *correctionEngine;

    Q_DISABLE_COPY(MImCorrectionHost)
};

#endif
