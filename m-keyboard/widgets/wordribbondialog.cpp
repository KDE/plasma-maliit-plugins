/* * This file is part of meego-keyboard-zh *
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

#include "wordribbondialog.h"
#include "wordribbondialogmodel.h"
#include "regiontracker.h"
#include <QtGui>
#include <QtCore>
#include <mdialog.h>
#include <mlayout.h>
#include <mlinearlayoutpolicy.h>
#include <mreactionmap.h>

#include "wordribbondialogview.h"

int WordRibbonCreator::totalCellcount = 0;

static int globalCanBarWidth = 0;

WordRibbonCreator::WordRibbonCreator(QObject *parent)
    :QObject(parent)
{
    size = QSize(480, 64);
}

QSizeF WordRibbonCreator::cellSize() const
{
    return size;
}

MWidget* WordRibbonCreator::createCell(const QModelIndex &index, MWidgetRecycler &recycler) const {
//    qDebug() <<Q_FUNC_INFO;
    WordRibbon *cell = qobject_cast<WordRibbon *>(recycler.take(WordRibbon::staticMetaObject.className()));
    if (cell == NULL) {
        cell = new WordRibbon(WordRibbon::DialogStyleMode, qobject_cast<MWidget*>(parent()));
        cell->setObjectName(QString("WordRibbonInFullDlgObj%1").arg(totalCellcount));
        cell->setViewType("WordRibbonType");
        cell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        cell->setProperty(MWidgetRecycler::RecycledObjectIdentifier, "WordRibbon");
        recycler.setMaxItemsPerClass(15);
        qDebug() << "WordRibbonType new cell" << ++totalCellcount;
        QObject::connect(cell, SIGNAL(itemClicked(QString, int)), this, SIGNAL(candidateClicked(QString)));
    }
    updateCell(index, cell);
    return cell;
}

void WordRibbonCreator::updateCell(const QModelIndex& index, MWidget * cell) const
{
//    qDebug() <<Q_FUNC_INFO;
    // Casting to MContentItem is safe because createCell will create only MContentItem(s).
    WordRibbon * candidateBar = qobject_cast<WordRibbon *>(cell);
    candidateBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVariant data = index.data(Qt::DisplayRole);
    // We are assuming here that model returns data in a QStringList: [<title>, <subtitle>, <pictureid>]
    QStringList rowData = data.value<QStringList>();

    candidateBar->setPreferredWidth(globalCanBarWidth);
    candidateBar->setMaximumWidth(globalCanBarWidth);
    candidateBar->repopulate(rowData);
}

WordRibbonDialog::WordRibbonDialog()
    :MDialog()
{
    setObjectName("WordRibbonDialogObj");
    setView(new WordRibbonDialogView(this));

    RegionTracker::instance().addRegion(*this);

    dataModel = new WordRibbonDialogModel(this);

    QGraphicsWidget *panel = centralWidget();
    listWidget = new MList(panel);

    MLayout *layout = new MLayout(panel);
    layout->setContentsMargins(0,0,0,0);
    panel->setLayout(layout);
    MLinearLayoutPolicy* landscapePolicy = new MLinearLayoutPolicy(layout, Qt::Vertical);
    MLinearLayoutPolicy* portraitPolicy = new MLinearLayoutPolicy(layout, Qt::Vertical);
    layout->setLandscapePolicy(landscapePolicy);
    layout->setPortraitPolicy(portraitPolicy);
    landscapePolicy->addItem(listWidget);
    portraitPolicy->addItem(listWidget);
    WordRibbonCreator *cellCreator = new WordRibbonCreator(this);
    QObject::connect(cellCreator, SIGNAL(candidateClicked(QString)),
                     this, SLOT(onRibbonClick(QString)));
    listWidget->setCellCreator(cellCreator);
    listWidget->setItemModel(dataModel);

    this->setModal(true);
}

WordRibbonDialog::~WordRibbonDialog()
{
}

void WordRibbonDialog::setCandidates(QStringList candidates, QString titleStr)
{
    this->titleString = titleStr;
    this->setTitle(titleString);

    globalCanBarWidth = ((WordRibbonDialogView*)view())->getWordRibbonDialogStyle()->dialogPreferredSize().width();

    qDebug() << "WordRibbonDialog::setCandidates calculate candidatesBar width =" << globalCanBarWidth
            << "this->boundingRect().width() =" << this->boundingRect().width()
            << "dialogLeftMargin =" << ((WordRibbonDialogView*)view())->getWordRibbonDialogStyle()->dialogLeftMargin()
            << "dialogRightMargin =" << ((WordRibbonDialogView*)view())->getWordRibbonDialogStyle()->dialogRightMargin()
            << "dialog-preferred-size =" << ((WordRibbonDialogView*)view())->getWordRibbonDialogStyle()->dialogPreferredSize().width();

    dataModel->setCandidates(candidates, globalCanBarWidth);

    if(this->candidatesList != candidates){
        this->candidatesList = candidates;
        this->listWidget->scrollTo(dataModel->index(0));
    }
}

void WordRibbonDialog::finalizeOrientationChange()
{
    this->setCandidates(this->candidatesList, this->titleString);
    qDebug() <<Q_FUNC_INFO <<" geometry = " <<this->geometry();
    emit geometryChanged();
}

void WordRibbonDialog::onRibbonClick(const QString & selectedWord)
{
    int index = this->candidatesList.indexOf(selectedWord);
    if (index >= 0) {
        emit candidateClicked(selectedWord, index);
    }
    this->accept();
}

void WordRibbonDialog::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view)
{
#ifndef HAVE_REACTIONMAP
    Q_UNUSED(reactionMap);
    Q_UNUSED(view);
    return;
#else
    Q_UNUSED(view);

    if (!isVisible())
        return;
    // full candidate take whole screen. Don't need reaction map.
    reactionMap->setInactiveDrawingValue();
    reactionMap->setTransform(QTransform());
    reactionMap->fillRectangle(0, 0, reactionMap->width(), reactionMap->height());
#endif // HAVE_REACTIONMAP
}
