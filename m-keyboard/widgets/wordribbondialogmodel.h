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


#ifndef WORDRIBBONDIALOGMODEL_H
#define WORDRIBBONDIALOGMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QStringList>
#include <QMap>
#include <QCache>

#include "wordribbon.h"

class WordRibbonDialogModel : public QAbstractListModel
{
    Q_OBJECT
public:
    WordRibbonDialogModel(QObject *parent = 0);
    ~WordRibbonDialogModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    void setCandidates(const QStringList &list, int rowWidth);
    QStringList candidates();

private:
    void fillCandidates(int rowWidth);

    QStringList listOrginal;
    typedef QList<QStringList> ListSeparate;
    QCache<int, ListSeparate> cache;

    WordRibbon *calcCandidatesBar;
    int currentRowWidth;
private:
    friend class Ut_WordRibbonDialogModel;
    friend class Ut_FullCandidate;
};

#endif // WORDRIBBONDIALOGMODEL_H
