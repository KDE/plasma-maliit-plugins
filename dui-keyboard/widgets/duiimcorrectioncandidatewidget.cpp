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



#include "duiimcorrectioncandidatewidget.h"
#include "duivirtualkeyboardstyle.h"

#include <QGraphicsSceneMouseEvent>
#include <QFontMetrics>
#include <QFont>
#include <QDebug>
#include <QString>

#include <DuiTheme>
#include <DuiSceneManager>
#include <duireactionmap.h>
#include <duiplainwindow.h>

namespace
{
    const int Margin = 10;
    const int CandidateMargin = 10; // Margin between two candidates
    const int ZValue = 10;
    const int CandidatesPreeditMargin = 10; // Margin between pre-edit rectangle and candidates
};


DuiImCorrectionCandidateWidget::DuiImCorrectionCandidateWidget(DuiVirtualKeyboardStyleContainer *style,
        QGraphicsWidget *parent) :
    DuiWidget(parent),
    rotationInProgress(false),
    activeWordIndex(0),
    width(0),
    height(0),
    row(-1),
    background(0),
    pos(new QPoint(0, 0)),
    styleContainer(style),
    fm(0),
    sceneManager(DuiPlainWindow::instance()->sceneManager())
{
    getStyleValues();
    setGeometry(0, 0, sceneManager->visibleSceneSize().width(),
                sceneManager->visibleSceneSize().height());

    // The z-value should always be more than vkb and text widget's z-value
    setZValue(ZValue);
}


DuiImCorrectionCandidateWidget::~DuiImCorrectionCandidateWidget()
{
    delete fm;
    fm = 0;
    delete pos;
    pos = 0;
    DuiTheme::releasePixmap(background);
}


void DuiImCorrectionCandidateWidget::setCandidates(const QStringList candidateList)
{
    qDebug() << __PRETTY_FUNCTION__ ;
    m_candidates.clear();
    m_candidates = candidateList;

    if (m_candidates.size() > 1) {
        QString string(m_candidates.at(m_candidates.size() - 1));

        if (string.compare(m_preeditString) == 0) {
            activeWordIndex = m_candidates.size() - 1;
            m_preeditString.clear();
        }
    }
    updateSize();
    update();
}


void DuiImCorrectionCandidateWidget::setPreeditString(const QString &string)
{
    m_preeditString = string;
}


QPoint DuiImCorrectionCandidateWidget::position() const
{
    return *pos;
}


QStringList DuiImCorrectionCandidateWidget::candidates() const
{
    return m_candidates;
}


QString DuiImCorrectionCandidateWidget::preeditString() const
{
    return m_preeditString;
}


void DuiImCorrectionCandidateWidget::setActiveIndex(int index)
{
    activeWordIndex = index;
}


void DuiImCorrectionCandidateWidget::updateSize()
{
    for (int i = 0; i < m_candidates.size(); i++) {
        int candidateWidth = fm->width(m_candidates.at(i));

        if (candidateWidth > width) {
            width = candidateWidth;
        }
    }
    height = ((m_candidates.size() - 1) * (fm->height() + CandidateMargin));
}


void DuiImCorrectionCandidateWidget::setPosition(const QPoint &position, int bottomLimit)
{
    QSize sceneSize = sceneManager->visibleSceneSize();
    int popupWidth = width + Margin;
    int popupHeight = height + Margin;

    if (bottomLimit < 0) {
        bottomLimit = sceneManager->visibleSceneSize().height();
    }

    *pos = position;

    // Adjust candidates list so that it doesn't
    // overlap with scene boundary, if possible.

    if (pos->x() + popupWidth > sceneSize.width())
        pos->setX(sceneSize.width() - popupWidth);

    if (pos->y() + popupHeight > bottomLimit)
        pos->setY(bottomLimit - popupHeight);

    if (pos->x() < 0)
        pos->setX(0);
    if (pos->y() < 0)
        pos->setY(0);

    update();
}


void DuiImCorrectionCandidateWidget::setPosition(const QRect &preeditRect, const int bottomLimit)
{
    qDebug() << "in " << __PRETTY_FUNCTION__;

    if (preeditRect.isNull() || !preeditRect.isValid()) {
        *pos = QPoint(0, 0);
        return;
    }

    QPoint position;
    QSize sceneSize = sceneManager->visibleSceneSize();
    int popupWidth = width + Margin;
    int popupHeight = height;

    // Set horizontal position

    if (preeditRect.right() + CandidatesPreeditMargin + popupWidth < sceneSize.width()) {
        // List is positioned to the right of pre-edit rectangle.
        position.setX(preeditRect.x() + preeditRect.width() + CandidatesPreeditMargin);
    } else if (preeditRect.x() - CandidatesPreeditMargin - popupWidth >= 0) {
        // List is positioned to the left of pre-edit rectangle.
        position.setX(preeditRect.x() - CandidatesPreeditMargin - popupWidth);
    } else {
        // No room in neither side. Pick one that has more.
        int roomRight = sceneSize.width() - preeditRect.right();
        int roomLeft = preeditRect.x();
        if (roomRight >= roomLeft) {
            // Align to right side of scene rect
            position.setX(sceneSize.width() - popupWidth);
        } else {
            // Align to left side of scene rect
            position.setX(0);
        }
    }

    // Set vertical position

    // Vertically the list is centered at the pre-edit rectangle.
    position.setY(preeditRect.y() + preeditRect.height() / 2 - popupHeight / 2);

    // Finally handle scene boundaries.
    setPosition(position, bottomLimit);
}


void DuiImCorrectionCandidateWidget::showWidget()
{
    show();

    // Extend overlay window to whole screen area.
    emit regionUpdated(mapRectToScene(QRect(QPoint(0, 0), sceneManager->visibleSceneSize())).toRect());
    emit opened();
}


void DuiImCorrectionCandidateWidget::paint(QPainter *painter,
        const QStyleOptionGraphicsItem *option,
        QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (m_candidates.size() > 1) {

        QRectF areaRect(pos->x(), pos->y(), width + Margin, height + Margin);

        if (background)
            painter->drawPixmap(areaRect, *background, background->rect());

        painter->setFont(font);

        painter->setPen(fontColor);
        int y = pos->y() + fm->height();

        for (int i = 0; i < m_candidates.size(); i++) {
            if (i == row) {
                painter->fillRect(pos->x(),
                                  (pos->y() + Margin + row *(fm->height() + CandidateMargin)),
                                  width + Margin, fm->height(), QBrush(candidateHighlightColor));
            }

            if (i != activeWordIndex) {
                painter->drawText(pos->x() + Margin / 2, y, m_candidates.at(i));
                y += fm->height() + CandidateMargin;
            }
        }
    }
}


void DuiImCorrectionCandidateWidget::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    if ((e->pos().x() > pos->x())  &&
            (e->pos().x() < (pos->x() + width + Margin)) &&
            (e->pos().y() < (pos->y() + height + Margin)) &&
            (e->pos().y() > pos->y())) {

        row = (e->pos().y() - pos->y()) / (fm->height() + CandidateMargin);

        if (row >= m_candidates.size() - 1)
            row = m_candidates.size() - 2;

        update();
    }
}


void DuiImCorrectionCandidateWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
    if ((e->pos().x() > pos->x()) &&
            (e->pos().x() < (pos->x() + width + Margin)) &&
            (e->pos().y() < (pos->y() + height + Margin)) &&
            (e->pos().y() > pos->y())) {

        row = (e->pos().y() - pos->y()) / (fm->height() + CandidateMargin);

        if (row >= m_candidates.size() - 1)
            row = m_candidates.size() - 2;

        QString candidate;

        if (row >= 0 && row < m_candidates.size() - 1) {
            if (row < activeWordIndex) {
                candidate = m_candidates.at(row);
                activeWordIndex = row;
            } else {
                candidate = m_candidates.at(row + 1);
                activeWordIndex = row + 1;
            }
        } else
            candidate.clear();

        emit candidateClicked(candidate);
    }

    hide();
    width = 0;
    return;
}


void DuiImCorrectionCandidateWidget::hideEvent(QHideEvent *event)
{
    DuiWidget::hideEvent(event);
    row = -1;
    emit hidden();
    emit regionUpdated(QRegion());
}


int DuiImCorrectionCandidateWidget::activeIndex()
{
    return activeWordIndex;
}


void DuiImCorrectionCandidateWidget::prepareToOrientationChange()
{
    if (isVisible()) {
        rotationInProgress = true;
        this->hide();
    }
}


void DuiImCorrectionCandidateWidget::finalizeOrientationChange()
{
    setGeometry(QRect(QPoint(0, 0), sceneManager->visibleSceneSize()));
    if (rotationInProgress) {
        showWidget();
        rotationInProgress = false;
    }
}


void  DuiImCorrectionCandidateWidget::getStyleValues()
{
    QString name;

    if (styleContainer) {
        name = style()->candidateBackground();
        DuiTheme::releasePixmap(background);
        background = 0;
        if (!name.isEmpty()) {
            background = DuiTheme::pixmap(name);
        }

        font = style()->font();
        delete fm;
        fm = new QFontMetrics(font);
        candidateHighlightColor = style()->candidateHighlightColor();
        float candidateHighlightOpacity = style()->candidateHighlightOpacity();
        candidateHighlightColor.setAlphaF(candidateHighlightOpacity);

        fontColor = style()->fontColor();
        float fontOpacity = style()->fontOpacity();
        fontColor.setAlphaF(fontOpacity);
    }
}

void DuiImCorrectionCandidateWidget::redrawReactionMaps()
{
    if (!scene()) {
        return;
    }

    int listHeight = ((m_candidates.size() - 1) * (fm->height() + CandidateMargin));
    QRectF areaRect(pos->x(), pos->y(), width + Margin, listHeight + Margin);

    foreach(QGraphicsView * view, scene()->views()) {
        DuiReactionMap *reactionMap = DuiReactionMap::instance(view);
        if (!reactionMap) {
            continue;
        }
        // Clear all with inactive color.
        reactionMap->setInactiveDrawingValue();
        reactionMap->setTransform(QTransform());
        reactionMap->fillRectangle(0, 0, reactionMap->width(), reactionMap->height());

        // Draw the actual candidate list area.
        reactionMap->setTransform(this, view);
        reactionMap->setReactiveDrawingValue();
        reactionMap->fillRectangle(areaRect);
    }
}

DuiVirtualKeyboardStyleContainer &DuiImCorrectionCandidateWidget::style()
{
    return *styleContainer;
}

