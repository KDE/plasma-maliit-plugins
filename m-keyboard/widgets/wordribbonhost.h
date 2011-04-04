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

#ifndef WORDRIBBONHOST_H
#define WORDRIBBONHOST_H

#include "reactionmappaintable.h"
#include "abstractenginewidgethost.h"

#include <QContiguousCache>
#include <QGraphicsWidget>
#include <QPointer>
#include <QString>
#include <MWidget>


class WordRibbon;
class WordRibbonDialog;
class MReactionMap;

class WordRibbonHost : public AbstractEngineWidgetHost, public ReactionMapPaintable
{
    Q_OBJECT

    friend class Ut_WordRibbonHost;

public:
    WordRibbonHost(MWidget *w, QObject* parent = 0);
    virtual ~WordRibbonHost();

    // Reimplement for AbstractEngineWidgetHost.
    /*! \reimp */
    virtual QGraphicsWidget *engineWidget() const;
    virtual QGraphicsWidget *inlineWidget() const;
    virtual bool isActive() const;
    virtual void setTitle(QString &title);
    virtual void setCandidates(const QStringList &candidates);
    virtual void appendCandidates(int startPos, const QStringList &candidate);
    virtual QStringList candidates() const;
    virtual void showEngineWidget(DisplayMode mode);
    virtual void hideEngineWidget();
    virtual DisplayMode displayMode() const;
    virtual void watchOnWidget(QGraphicsWidget *widget);
    virtual void setPosition(const QRect &cursorRect);
    virtual void handleNavigationKey(NaviKey key);
    virtual int suggestedWordIndex() const;
    virtual void prepareToOrientationChange();
    virtual void finalizeOrientationChange();
    virtual void reset();
    virtual void setPageIndex(int index);
    /*! \reimp_end */

    // Reimplement for ReactionMapPaintable.
    /*! \reimp */
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);
    bool isPaintable() const;
    bool isFullScreen() const;
    /*! \reimp_end */

private:
    void clearCandidate();
    bool isEmpty();
    void fetchMoreCandidates();
    
private slots:
    void updatePosition();
    void openWordRibbonDialog();
    void handleDialogClosed();

signals:
    void sizeUpdated(const QSize );

private:
    QList<QPointer<QGraphicsWidget> > watchedWidgets;
    WordRibbon* wordRibbon;
    WordRibbonDialog* ribbonDialog;
    int uniqueIndex;
    QString dialogTitle;
    QContiguousCache<QString> candidatesCache;
};

#endif
