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

#include <QObject>
#include <QPoint>
#include <QString>
#include <QStringList>
#include <QRect>
#include <QRegion>

class MSceneWindow;
class MReactionMap;
class MImWordTracker;
class MImWordList;
class MReactionMap;
class QGraphicsView;

/*!
  \class MImCorrectionHost
  \brief The MImCorrectionHost class is used to show error correction
  candidate word tracker or word list.
*/
class MImCorrectionHost : public QObject
{
    Q_OBJECT

    friend class Ut_MImCorrectionHost;

public:
    //! CandidateMode is used by showCorrectionWidget and setMode.
    enum CandidateMode {
        WordTrackerMode,  //!< word tracker
        WordListMode      //!< word suggestion list
    };

    /*! Constructor
     *
     */
    explicit MImCorrectionHost(MSceneWindow *parentWindow);

    /*! Destructor
     *
     */
    ~MImCorrectionHost();

    /*!
     * \brief Returns true if tracker or word list is active.
     */
    bool isActive() const;

    /*! Set the candidate list
     *
     */
    void setCandidates(QStringList candidate);

    /*!
     * \brief Shows candidate widget with different \a mode.
     *
     * \sa CandidateMode.
     */
    virtual void showCorrectionWidget(CandidateMode mode = WordTrackerMode);

    /*!
     * \brief Hides candidate widget with or without animation according \a withAnimation.
     *
     * \param withAnimation if ture hide widget by useing animation.
     */
    virtual void hideCorrectionWidget(bool withAnimation = true);

    /*!
     * \brief Returns current used mode for the candidate widget.
     *
     * \sa CandidateMode.
     */
    CandidateMode candidateMode() const;

    /*! \brief Sets the position of word tracker.
     *
     * The word tracker cannot be outside screen.
     */
    void setPosition(const QPoint &pos);

    /*!
     * \brief Sets the position of word tracker based on pre-edit rectangle.
     *
     * TODO: this method will be removed. Will only support to set word tracker position
     * according the cursor position, not base on pre-edit rectangle anymore.
     */
    void setPosition(const QRect &preeditRect);

    /*!
     * \brief Returns the suggested word.
     *
     * The suggestion is the word present on word tracker or the one clicked by user in
     * the word list.
     */
    QString suggestion() const;

    /*! Returns the actual position
     * \return QPoint Returns the actual set position.
     */
    QPoint position() const;

    /*!
     * \brief Draw its reactive areas onto the reaction map
     */
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

    //! Prepare virtual keyboard for orientation change
    void prepareToOrientationChange();

    //! Finalize orientation change
    void finalizeOrientationChange();

    //! Clear stored suggestion and hide candidate widget.
    void reset();

signals:
    //! Updates the preedit word
    void candidateClicked(const QString &);

    //! Updates the screen region used by the widget
    void regionUpdated(const QRegion &);

protected slots:

    void handleCandidateClicked(const QString &candidate);

    void sendRegion();

private:
    bool rotationInProgress;
    QPoint wordTrackerPosition;
    QStringList candidates;
    CandidateMode currentMode;
    QString suggestionString;

    MImWordTracker *wordTracker;
    MImWordList *wordList;

    Q_DISABLE_COPY(MImCorrectionHost)
};

#endif
