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

#include "wordribbondialogmodel.h"
#include "wordribbon.h"

#include <QDebug>
#include <QtCore>

WordRibbonDialogModel::WordRibbonDialogModel(QObject *parent) :
    QAbstractListModel(parent),
    calcCandidatesBar(0),
    currentRowWidth(0)
{
    setObjectName("WordRibbonDialogModelObj");
    calcCandidatesBar = new WordRibbon(WordRibbon::DialogStyleMode);
}

WordRibbonDialogModel::~WordRibbonDialogModel()
{
    if(calcCandidatesBar)
        delete calcCandidatesBar;
}

int WordRibbonDialogModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (cache.contains(this->currentRowWidth))
        return cache.object(this->currentRowWidth)->count();
    else
        return 0;
}

QVariant WordRibbonDialogModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        if (!cache.contains(this->currentRowWidth) ||
            cache.object(this->currentRowWidth)->count() <= index.row()) {
            return QVariant();
        }
        QStringList rowData = cache.object(this->currentRowWidth)->at(index.row());
        return QVariant(rowData);
    }
    return QVariant();
}

void WordRibbonDialogModel::fillCandidates(int rowWidth)
{
    ListSeparate *listSeparate = new ListSeparate;

    int index = 0;
    int consume = 0;
    int i = 0;
    int tmpConsume = 0;

    do {
       tmpConsume = calcCandidatesBar->capacity(rowWidth, listOrginal, index);
        if(tmpConsume <= 0){
            delete listSeparate;
            return;
        }
        consume += tmpConsume;
        listSeparate->append(QStringList(listOrginal.mid(index, tmpConsume)));
        index = consume;
        ++i;
    } while (consume < listOrginal.count());
    cache.insert(rowWidth, listSeparate);
    currentRowWidth = rowWidth;
    reset();
}

void WordRibbonDialogModel::setCandidates(const QStringList &list, int rowWidth)
{
    if(rowWidth <= 0 || list.empty()) {
        if (rowWidth <= 0)
            qWarning() << Q_FUNC_INFO << "rowWidth <= 0";
        if (list.empty())
            qWarning() << Q_FUNC_INFO << "list.empty()";
        listOrginal.clear();
        cache.clear();
        currentRowWidth = 0;
        return;
    }

    if (listOrginal != list) {
        listOrginal = list;
        cache.clear();
        currentRowWidth = 0;
        fillCandidates(rowWidth);
        return;
    }
    else {
        if (cache.contains(rowWidth)) {
            if(currentRowWidth != rowWidth) {
                currentRowWidth = rowWidth;
                reset();
            }
            return;
        }
        else {
            fillCandidates(rowWidth);
            return;
        }
    }
}

QStringList WordRibbonDialogModel::candidates()
{
    return listOrginal;
}
