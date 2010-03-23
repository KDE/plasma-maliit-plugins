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

#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QString>
#include <QFontMetrics>

#include <DuiSceneManager>
#include <duireactionmap.h>
#include <duiplainwindow.h>
#include <DuiContentItem>
#include <DuiWidgetRecycler>
#include <DuiList>
#include <DuiLabel>
#include <QGraphicsLinearLayout>
#include <QStringListModel>

namespace
{
    const int ZValue = 10;
    const int CandidatesPreeditMargin = 10; // Margin between pre-edit rectangle and candidates
    const int MaximumCandidateLength = 100; // Maximum length of a candidate.
    const QString CandidatesListObjectName("CorrectionCandidateList");
    const QString CandidatesItemObjectName("CorrectionCandidateItem");
    const QString CandidatesItemLabelObjectName("CorrectionCandidateItemTitle");
};

class DuiImCorrectionContentItemCreator : public DuiAbstractCellCreator<DuiContentItem>
{
public:
    DuiImCorrectionContentItemCreator();
    /*! \reimp */
    virtual DuiWidget *createCell(const QModelIndex &index, DuiWidgetRecycler &recycler) const;
    virtual void updateCell(const QModelIndex &index, DuiWidget *cell) const;
    virtual QSizeF cellSize() const;
    /*! \reimp_end */
private:
    void updateContentItemMode(const QModelIndex &index, DuiContentItem *contentItem) const;

    QSizeF size;
};

DuiImCorrectionContentItemCreator::DuiImCorrectionContentItemCreator()
{
    // Initializes size
    DuiContentItem cell(DuiContentItem::SingleTextLabel);
    cell.setObjectName(CandidatesItemObjectName);
    size = cell.effectiveSizeHint(Qt::PreferredSize);
}

DuiWidget *DuiImCorrectionContentItemCreator::createCell(const QModelIndex &index, DuiWidgetRecycler &recycler) const
{
    DuiWidget *cell = recycler.take(DuiContentItem::staticMetaObject.className());
    if (cell == NULL) {
        cell = new DuiContentItem(DuiContentItem::SingleTextLabel);
        cell->setObjectName(CandidatesItemObjectName);
    }
    updateCell(index, cell);
    return cell;
}

void DuiImCorrectionContentItemCreator::updateCell(const QModelIndex &index, DuiWidget *cell) const
{
    if (cell == NULL)
        return;
    DuiContentItem *contentItem = qobject_cast<DuiContentItem *>(cell);
    const QVariant data = index.data(Qt::DisplayRole);
    const QStringList rowData = data.value<QStringList>();
    if (rowData.size() > 0) {
        // Restrict the candidate length to MaximumCandidateLength characters.
        contentItem->setTitle(rowData[0].left(MaximumCandidateLength));
    }
    updateContentItemMode(index, contentItem);
}

QSizeF DuiImCorrectionContentItemCreator::cellSize() const
{
    return size;
}

void DuiImCorrectionContentItemCreator::updateContentItemMode(const QModelIndex &index,
        DuiContentItem *contentItem) const
{
    const int row = index.row();
    bool thereIsNextRow = index.sibling(row + 1, 0).isValid();
    if (row == 0) {
        contentItem->setItemMode(DuiContentItem::SingleColumnTop);
    } else if (thereIsNextRow) {
        contentItem->setItemMode(DuiContentItem::SingleColumnCenter);
    } else {
        contentItem->setItemMode(DuiContentItem::SingleColumnBottom);
    }
}

DuiImCorrectionCandidateWidget::DuiImCorrectionCandidateWidget(QGraphicsWidget *parent)
    : DuiWidget(parent),
      rotationInProgress(false),
      candidatePosition(0, 0),
      fontMetrics(0),
      sceneManager(DuiPlainWindow::instance()->sceneManager()),
      containerWidget(new DuiWidget(this)),
      candidatesWidget(new DuiList(containerWidget)),
      cellCreator(new DuiImCorrectionContentItemCreator),
      candidatesModel(new QStringListModel()),
      candidateWidth(0)
{
    setGeometry(0, 0, sceneManager->visibleSceneSize().width(),
                sceneManager->visibleSceneSize().height());

    // The z-value should always be more than vkb and text widget's z-value
    setZValue(ZValue);

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout;
    containerWidget->setLayout(layout);
    candidatesWidget = new DuiList(containerWidget);

    layout->addItem(candidatesWidget);
    candidatesWidget->setObjectName(CandidatesListObjectName);
    candidatesWidget->setCellCreator(cellCreator);
    candidatesWidget->setItemModel(candidatesModel);
    connect(candidatesWidget, SIGNAL(itemClicked(const QModelIndex &)), this, SLOT(select(const QModelIndex &)));

    //TODO: This is a hack way to get font information from DuiList.
    //Could be refined later if DuiList provides other better way.
    DuiLabel label;
    label.setObjectName(CandidatesItemLabelObjectName);
    fontMetrics  = new QFontMetrics(label.font());
}


DuiImCorrectionCandidateWidget::~DuiImCorrectionCandidateWidget()
{
    delete fontMetrics;
    fontMetrics = 0;
    delete candidatesModel;
}

void DuiImCorrectionCandidateWidget::setCandidates(const QStringList candidateList)
{
    // Filter the preedit from the candidate list.
    QStringList filteredCandidateList = candidateList;
    for (int i = 0; i < filteredCandidateList.size(); i++) {
        if (m_preeditString.compare(filteredCandidateList.at(i), Qt::CaseInsensitive) == 0) {
            filteredCandidateList.removeAt(i);
            break;
        }
    }

    // Below is the QT way to update model size
    if (candidatesModel->rowCount() > 0)
        candidatesModel->removeRows(0, candidatesModel->rowCount());
    candidatesModel->insertRows(0, filteredCandidateList.size());
    candidatesModel->setStringList(filteredCandidateList);

    // Calculate the width for DuiConentItem dynamically.
    // To ensure the whole words in candidate list could be shown.
    candidateWidth = 0;
    foreach (const QString &candidate, filteredCandidateList) {
        candidateWidth = qMax(candidateWidth, fontMetrics->width(candidate));
    }
    candidateWidth += candidatesWidget->preferredWidth();
}

void DuiImCorrectionCandidateWidget::setPreeditString(const QString &string)
{
    m_preeditString = string;
}

QPoint DuiImCorrectionCandidateWidget::position() const
{
    return candidatePosition;
}

QStringList DuiImCorrectionCandidateWidget::candidates() const
{
    return candidatesModel->stringList();
}

QString DuiImCorrectionCandidateWidget::preeditString() const
{
    return m_preeditString;
}

void DuiImCorrectionCandidateWidget::setPosition(const QPoint &position, int bottomLimit)
{
    qDebug() << __PRETTY_FUNCTION__;
    QSize sceneSize = sceneManager->visibleSceneSize();
    int popupWidth = candidateWidth;
    int popupHeight = candidatesWidget->preferredSize().height();
    if (bottomLimit < 0) {
        bottomLimit = sceneManager->visibleSceneSize().height();
    }

    candidatePosition = position;

    // Adjust candidates list so that it doesn't
    // overlap with scene boundary, if possible.

    if (candidatePosition.x() + popupWidth > sceneSize.width())
        candidatePosition.setX(sceneSize.width() - popupWidth);

    if (candidatePosition.y() + popupHeight > bottomLimit)
        candidatePosition.setY(bottomLimit - popupHeight);

    if (candidatePosition.x() < 0)
        candidatePosition.setX(0);
    if (candidatePosition.y() < 0)
        candidatePosition.setY(0);

    containerWidget->setPos(candidatePosition.x(), candidatePosition.y());
}

void DuiImCorrectionCandidateWidget::setPosition(const QRect &preeditRect, const int bottomLimit)
{
    qDebug() << "in " << __PRETTY_FUNCTION__;

    if (preeditRect.isNull() || !preeditRect.isValid()) {
        candidatePosition = QPoint(0, 0);
        return;
    }

    QPoint position;
    QSize sceneSize = sceneManager->visibleSceneSize();
    int popupWidth = candidateWidth;
    int popupHeight = candidatesWidget->preferredSize().height();

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

    // Vertically the candidatesWidget is centered at the pre-edit rectangle.
    position.setY(preeditRect.y() + preeditRect.height() / 2 - popupHeight / 2);

    // Finally handle scene boundaries.
    setPosition(position, bottomLimit);
}

void DuiImCorrectionCandidateWidget::showWidget()
{
    // The hight of DuiList is automatically expanded.
    // But the width of DuiList is not automatically expanded.
    // So set the container widget's width to candidateWidth,
    // to make DuiList have the enough width to show whole words.
    containerWidget->setPreferredWidth(candidateWidth);
    show();

    // Extend overlay window to whole screen area.
    emit regionUpdated(mapRectToScene(QRect(QPoint(0, 0), sceneManager->visibleSceneSize())).toRect());
    emit opened();
}

void DuiImCorrectionCandidateWidget::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    Q_UNUSED(e);
}

void DuiImCorrectionCandidateWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
    Q_UNUSED(e);
    hide();
}

void DuiImCorrectionCandidateWidget::select(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    const QVariant selectedVariant = candidatesModel->data(index, Qt::DisplayRole);
    Q_ASSERT(selectedVariant.isValid());
    const QString candidate = selectedVariant.toString();
    if (candidate != m_preeditString) {
        emit candidateClicked(candidate);
    }
    hide();
}

void DuiImCorrectionCandidateWidget::hideEvent(QHideEvent *event)
{
    DuiWidget::hideEvent(event);
    emit hidden();
    emit regionUpdated(QRegion());
}

int DuiImCorrectionCandidateWidget::activeIndex() const
{
    int activeWordIndex = -1;
    QStringList candidateList = candidatesModel->stringList();
    for (int i = 0; i < candidateList.size(); i++) {
        if (m_preeditString.compare(candidateList.at(i), Qt::CaseInsensitive) == 0) {
            activeWordIndex = i;
            break;
        }
    }
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

void DuiImCorrectionCandidateWidget::redrawReactionMaps()
{
    if (!scene()) {
        return;
    }

    foreach(QGraphicsView * view, scene()->views()) {
        DuiReactionMap *reactionMap = DuiReactionMap::instance(view);
        if (!reactionMap) {
            continue;
        }
        // Clear all with inactive color.
        reactionMap->setInactiveDrawingValue();
        reactionMap->setTransform(QTransform());
        reactionMap->fillRectangle(0, 0, reactionMap->width(), reactionMap->height());

        // Draw the actual candidate candidatesWidget area.
        reactionMap->setTransform(this, view);
        reactionMap->setReactiveDrawingValue();
        reactionMap->fillRectangle(candidatesWidget->geometry());
    }
}
