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

class QGraphicsLinearLayout;
class MSeparator;
class MImWordListItem;
class MReactionMap;

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
     *
     * The first candidate in the list must be the word the user typed in.
     * typedWordIsInDictionary indicates if this word is already present in a dictionary.
     */
    void setCandidates(const QStringList &candidates, bool typedWordIsInDictionary);

    /*!
     * \brief Returns the suggestion candidates shown in the widget.
     *
     * The list is limited to MaxCandidateCount items.
     */
    QStringList candidates() const;

    /*!
     * \brief Draw its reactive areas onto the reaction map
     */
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

signals:
    /*!
     * \brief This signal is emitted when clicking on a candidate.
     */
    void candidateClicked(const QString &);

private slots:

    void select();

private:
    QStringList mCandidates;
    QGraphicsLinearLayout *mainLayout;
    MImWordListItem *candidateItems[MaxCandidateCount];
    MImWordListItem *addToDictionaryItem;
    MSeparator *addToDictionarySeparator;
};

#endif
