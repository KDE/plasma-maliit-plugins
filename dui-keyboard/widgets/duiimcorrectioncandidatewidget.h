/* * This file is part of dui-keyboard *
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



#ifndef DUIIMCORRECTIONCANDIDATEWIDGET_H
#define DUIIMCORRECTIONCANDIDATEWIDGET_H

#include <QModelIndex>
#include <DuiWidget>

class DuiSceneManager;
class DuiList;
class DuiImCorrectionContentItemCreator;
class QStringListModel;
class QFontMetrics;

/*!
  \class DuiImCorrectionCandidateWidget
  \brief The DuiImCorrectionCandidateWidget class is used to show error correction candidate list
*/
class DuiImCorrectionCandidateWidget: public DuiWidget
{
    Q_OBJECT

    friend class Ut_DuiImCorrectionCandidateWidget;

public:
    /*! Constructor
     *
     */
    explicit DuiImCorrectionCandidateWidget(QGraphicsWidget *parent = 0);

    /*! Destructor
     *
     */
    ~DuiImCorrectionCandidateWidget();

    /*! Set the candidate list
     *
     */
    void setCandidates(QStringList candidate);

    virtual void showWidget();

    /*! \brief Sets the position of candidate list. The list cannot be outside screen.
     *
     * If \a bottomLimit is provided, it is respected but not at the expense of
     * being out of screen.
     */
    void setPosition(const QPoint &pos, int bottomLimit = -1);

    /*!
     * \brief Sets the position of candidate list based on pre-edit rectangle.
     *
     * Candidate list is put horizontally next to the pre-edit rectangle. Right side
     * of the rectangle is preferred but left side will be used if there is not enough
     * space. Vertically list will be in the middle of the rectangle. If \a bottomLimit
     * is provided, it is respected but not at the expense of being out of screen.
     */
    void setPosition(const QRect &preeditRect, int bottomLimit = -1);

    /*! Returns the index of preedit string in the candidate list.
     */
    int activeIndex() const;

    /*! Sets the preedit string
     */
    void setPreeditString(const QString &);

    /*! Returns the actual position
     * \return QPoint Returns the actual set position
     */
    QPoint position() const;

    /*! Returns the actual set candidates
     */
    QStringList candidates() const;

    /*! Returns the Preedit String
     */
    QString preeditString() const;

    /*!
     * Draw its reactive areas onto the reaction maps
     */
    void redrawReactionMaps();

    //! Prepare virtual keyboard for orientation change
    void prepareToOrientationChange();

    //! Finalize orientation change
    void finalizeOrientationChange();

signals:
    /*! Updates the preedit word
     */
    void candidateClicked(const QString &);

    /*! Updates the screen region used by the widget
     */
    void regionUpdated(const QRegion &);

    /*! Emitted when it hides itself.
     */
    void hidden();

    //! Emitted when visible and showing the list.
    void opened();

protected:
    /*! \reimp */
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);
    virtual void hideEvent(QHideEvent *event);
    /*! \reimp_end */

protected slots:
    void select(const QModelIndex &);

private:
    bool rotationInProgress;
    QString m_preeditString;
    QPoint candidatePosition;
    QFontMetrics *fontMetrics;
    DuiSceneManager *sceneManager;
    DuiWidget *containerWidget;
    DuiList *candidatesWidget;
    DuiImCorrectionContentItemCreator *cellCreator;
    QStringListModel *candidatesModel;
    int candidateWidth;

    Q_DISABLE_COPY(DuiImCorrectionCandidateWidget)
};

#endif
