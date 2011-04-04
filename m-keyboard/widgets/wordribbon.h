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


#ifndef WORDRIBBON_H
#define WORDRIBBON_H

#include <MStylableWidget>
#include "wordribbonstyle.h"
#include "wordribbonhost.h"

class WordRibbonItem;
class MButton;
class MReactionMap;

/*!
 * \brief WordRibbon is a widget which displays a list of candidate items
 */
class WordRibbon: public MStylableWidget
{
    Q_OBJECT
    M_STYLABLE_WIDGET(WordRibbonStyle)

    friend class Ut_WordRibbon;

public:
    /*!
     * \brief The style mode used to display the item
     */
    enum ItemStyleMode { 
        RibbonStyleMode, //! Style when the items is displayed inline
        DialogStyleMode  //! Style when the items is displayed in a dialog
    };

    WordRibbon(ItemStyleMode mode, MWidget* parent = 0);
    virtual ~WordRibbon();
    //! reimp
    virtual void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);
    //! reimp_end

    /*!
     * \brief Repopulates the item list
     */
    void repopulate(const QStringList &newWordRibbon);

    /*!
     * \brief Removes all the items from the list
     */
    void removeAllItems();

    /*!
     * \brief Returns whether the list is empty or not
     */
    bool isEmpty() const;

    /*!
     * \brief Highlights an item as specified in the index
     */
    void setHighlightedItem(int index);

    /*!
     * \brief Clears highlighted item
     */
    void clearHighlightedItem();

    /*!
     * \brief Handles navigation key
     */
    void handleNavigationKey(WordRibbonHost::NaviKey key);

    /*!
     * \brief Returns the number of items that the ribbon could fit
     * \param width The allocated width
     * \param candidateList The new list of the candidate that would be fit into
     * \param startPos The starting position in the list
     */
    int capacity(int width, QStringList candidateList, int startPos);
public slots:
    void finalizeOrientationChange();

signals:
    /*!
     * \brief Emitted when an item is clicked
     * \param label The label of the item
     * \param index The index of the item on the list
     */
    void itemClicked(const QString &label, int index);

    /*!
     * \brief Emitted when more candidates is requested
     * by pressing more candidate button
     */
    void moreCandidatesRequested();

    /*!
     * \brief Emitted when the size of the ribbon is changed
     */
    void sizeChanged(QSize);

private slots:
    void handleItemPressed();
    void handleItemClicked();

private:
    void initializeSubWidgets();
    void updatePrepareShowingWidget();
    void clearAllFlags();

private:
    // Sub widget and their own position
    QList<WordRibbonItem* > itemList;
    int numCreatedItems;

    QRectF contentRect;

    MButton* moreButton;

    int highlightItemIndex;
    ItemStyleMode mode;
};

#endif // WORDRIBBON_H
