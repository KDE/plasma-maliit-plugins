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


#ifndef MIMWORDLIST_H
#define MIMWORDLIST_H

#include <MDialog>
#include "mimoverlay.h" 

class QGraphicsLinearLayout;
class MContentItem;
class MImWordListItem;
class MImWordList;

/*!
 * \brief MIMWordListWindow is used as the plain translucent parent window for word list dialog.
 *
  * MIMWordListWindow prevents mouse and touch events from reaching the virtual keyboard or the application.
  * \sa MImOverlay.
 */
class MIMWordListWindow : public MImOverlay
{
    Q_OBJECT
public:
    //! Constructor
    explicit MIMWordListWindow(MImWordList *widget);

public slots:
    /*
     * \brief This slot is connected with word list widget's appeared() signal.
     */
    void handleListAppeared();

    /*
     * \brief This slot is connected with word list widget's disappeared() signal.
     */
    void handleListDisappeared();

protected:
    /*! \reimp */
    virtual bool sceneEvent(QEvent *event);
    /*! \reimp_end */

private:
    MImWordList *listWidget;
};

/*!
 * \brief MIMWordList is used for word list dialog.
 *
 * MImWordList shows a dialog which list the suggested candidates.
 */
class MImWordList : public MDialog
{
    Q_OBJECT
    friend class Ut_MImWordList;
    friend class Ut_MImCorrectionCandidateWidget;

public:
    enum {
        MaxCandidateCount = 5
    };

    //! Constructor
    explicit MImWordList();

    //! Destructor
    virtual ~MImWordList();

    /*!
     * \brief Sets suggestion candidates.
     */
    void setCandidates(const QStringList &candidates);

    /*!
     * \brief Returns the suggestion candidates.
     */
    QStringList candidates() const;

    /*!
     * \brief Sets the highlight suggestion candidate.
     */
    void setHighlightCandidate(const QString &);

    /*!
     * \brief Returns the occupied region of word list widget.
     */
    QRegion region() const;

signals:
    /*!
     * \brief This signal is emitted when clicking on a candidate.
     */
    void candidateClicked(const QString &);

    //! Emitted when the occupied region of word list is changed.
    void regionChanged();

private slots:

    void select();

    void handleVisibilityChanged();

private:
    QStringList mCandidates;
    QGraphicsLinearLayout *mainLayout;
    MImWordListItem *candidateItems[MaxCandidateCount];
    MIMWordListWindow *parentWindow;
};

#endif
