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


#ifndef WORDRIBBONDIALOG_H
#define WORDRIBBONDIALOG_H

#include <MDialog>
#include <MList>
#include <MAbstractCellCreator>

#include "wordribbon.h"
#include "reactionmappaintable.h"

class WordRibbonDialogSwitcher;
class WordRibbonDialogModel;

class WordRibbonCreator : public QObject, public MCellCreator
{
    Q_OBJECT
private:
    QSizeF size;
public:
    static int totalCellcount;
    WordRibbonCreator(QObject *parent=0);
    virtual QSizeF cellSize() const;
    virtual MWidget *createCell(const QModelIndex &index, MWidgetRecycler &recycler) const;
    virtual void updateCell(const QModelIndex &index, MWidget *cell) const;

signals:
    void candidateClicked(const QString & selectedWord);
};

class WordRibbonDialog : public MDialog
{
    Q_OBJECT
    static const int DefaultCandidateTextbuttonCount = 20;
public:
    WordRibbonDialog();
    virtual ~WordRibbonDialog();

    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

public slots:
    void setCandidates(QStringList candidates, QString titleStr = "");
    void finalizeOrientationChange(); 

signals:
    void candidateClicked(const QString & selectedWord, int wordIndex);

private:
    WordRibbonDialogModel *dataModel;
    //M::Orientation orientation;
    QString titleString;
    MList   *listWidget;
    QStringList candidatesList;

private slots:
    void onRibbonClick(const QString & selectedWord);

private:
    friend class Ut_WordRibbonDialog;
};

#endif // WORDRIBBONDIALOG_H
