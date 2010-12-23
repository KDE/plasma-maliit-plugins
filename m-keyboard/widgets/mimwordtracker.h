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



#ifndef MIMWORDTRACKER_H
#define MIMWORDTRACKER_H

#include <MStylableWidget>
#include "mimwordtrackerstyle.h"

#include <QTimeLine>

class QGraphicsWidget;
class QGraphicsLinearLayout;
class MImCorrectionCandidateItem;
class MSceneWindow;
class MReactionMap;

/*!
 * \brief The MImWordTracker class is used to show error correction word tracker.
 */
class MImWordTracker : public MStylableWidget
{
    Q_OBJECT
    friend class Ut_MImWordTracker;
    friend class Ut_MImCorrectionCandidateWidget;

public:
    //! Constructor
    explicit MImWordTracker(MSceneWindow *parentWindow);

    //! Destructor
    ~MImWordTracker();

    /*!
     * \brief Set suggested candidate.
     */
    void setCandidate(const QString &);

    /*!
     * \brief Returns the suggested candidate.
     */
    QString candidate() const;

    /*!
     * \brief Returns the ideal width of the word tracker.
     *
     * The ideal width is the actually used width of the word tracker together with margins and paddings.
     */
    qreal idealWidth() const;

    /*!
     * \brief Returns the height of pointer for word tracker.
     */
    qreal pointerHeight() const;

    /*!
     * \brief Appears word tracker widget with or without default animation.
     */
    void appear(bool withAnimation = false);
    
    /*!
     * \brief Disappear word tracker widget with or without default animation.
     */
    void disappear(bool withAnimation = false);

    /*!
     * \brief Sets the position for word tracker according cursor rectangle.
     */
    void setPosition(const QRect &cursorRect);

    /*!
     * \brief Returns the visible region of word tracker.
     */
    QRegion region() const;

    /*!
     * \brief Draw its reactive areas onto the reaction map
     */
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

signals:
    //! Emitted when word tracker is clicked.
    void candidateClicked(const QString &);

    /*!
     * \brief The signal is emitted when the word tracker has been tapped and holded.
     */
    void longTapped();

    //! Emitted when the occupied region of word tracker is changed.
    void regionChanged();

protected slots:
    void select();

    void longTap();

    /*!
     * Method to fade the vkb during transition
     */
    void fade(int);

    /*!
     * This function gets called when fading is finished
     */
    void showHideFinished();

    /*!
     * Update stored style stuffs when the theme changed.
     */
    void onThemeChangeCompleted();

protected:
    /*! \reimp */
    virtual void drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option) const;
    /*! \reimp_end */

private:
    void setupTimeLine();

    QGraphicsWidget *containerWidget;
    QString mCandidate;
    int mIdealWidth;
    QGraphicsLinearLayout *mainLayout;
    MImCorrectionCandidateItem *candidateItem;
    QTimeLine showHideTimeline;
    qreal pointerXOffset;

    M_STYLABLE_WIDGET(MImWordTrackerStyle)
};

#endif
