/* * This file is part of m-keyboard *
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



#include "mvirtualkeyboardstyle.h"
#include "mbuttonarea.h"
#include "flickupbutton.h"
#include "limitedtimer.h"
#include "popupbase.h"
#include <QDebug>
#include <QEvent>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QTextLayout>
#include <QTextLine>
#include <MApplication>
#include <MComponentData>
#include <MFeedbackPlayer>
#include <mreactionmap.h>
#include <MScalableImage>
#include <MTheme>

MButtonArea::MButtonArea(MVirtualKeyboardStyleContainer *style,
                         QSharedPointer<const LayoutSection> sectionModel,
                         ButtonSizeScheme buttonSizeScheme,
                         bool usePopup,
                         QGraphicsWidget *parent)
    : KeyButtonArea(style, sectionModel, buttonSizeScheme, usePopup, parent),
      mainLayout(*new QGraphicsLinearLayout(Qt::Vertical, this)),
      buttons(sectionModel->keyCount())
{
    // This call in addition to sceneEventFilter enables KeyButtonArea to work correctly with
    // mouse events. Otherwise MButtons and MLabels would get them.
    setFiltersChildEvents(true);

    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    loadKeys();
}

MButtonArea::~MButtonArea()
{
    // Release any key that might be pressed before destroying them.
    setActiveKey(0);

    setLayout(0);
    qDeleteAll(buttons);
}


bool MButtonArea::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    bool stopPropagation = false;

    // FlickUpButtons contain labels which are also filtered but we don't want them.
    if (dynamic_cast<FlickUpButton *>(watched)) {
        if (event->type() == QEvent::GrabMouse) {
            // Let's control the mouse events from now on.
            grabMouse();
        } else if (event->type() == QEvent::GraphicsSceneMousePress) {
            // We still need the press event.
            // Map mouse coordinate to our coordinates.
            QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
            mouseEvent->setPos(mapFromItem(watched, mouseEvent->pos()));
            sceneEvent(mouseEvent);

            stopPropagation = true;
        }
    }
    return stopPropagation;
}

void MButtonArea::drawReactiveAreas(MReactionMap *reactionMap, QGraphicsView *view)
{
    reactionMap->setTransform(this, view);
    reactionMap->setReactiveDrawingValue();

    const int numRows = rowCount();

    for (int row = 0; row < numRows; ++row) {
        QRectF rowRect(mainLayout.itemAt(row)->geometry());

        // Expand the row rectangle in height by the amount of reactive margin of buttons.
        const qreal reactiveMargin = (buttons.first()->buttonBoundingRect().height() - rowRect.height()) / 2.0f;
        rowRect.adjust(0, -reactiveMargin, 0, reactiveMargin);

        reactionMap->fillRectangle(rowRect.toRect());
    }
}

void MButtonArea::loadKeys()
{
    // Init layout containing the rows.
    mainLayout.setSpacing(static_cast<qreal>(style()->spacingVertical()));
    mainLayout.setContentsMargins(0, 0, 0, 0);

    QVector<FlickUpButton *>::iterator nextFreeButton(buttons.begin());
    const int numRows = rowCount();

    for (int row = 0; row != numRows; ++row) {
        const int numColumns = sectionModel()->columnsAt(row);

        QGraphicsLinearLayout *rowLayout = new QGraphicsLinearLayout(Qt::Horizontal, &mainLayout);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(style()->spacingHorizontal());

        // Add buttons
        for (int col = 0; col < numColumns; ++col, ++nextFreeButton) {

            // Parameters to fetch from base class.
            const VKBDataKey *dataKey;
            QSize buttonSize;
            bool stretchesHorizontally;
            buttonInformation(row, col, dataKey, buttonSize, stretchesHorizontally);

            FlickUpButton *button = new FlickUpButton(*dataKey);

            if (dataKey->binding()->action() == KeyBinding::ActionShift) {
                shiftButton = button;
            }

            if (stretchesHorizontally) {
                button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            } else {
                button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
            }
            button->setPreferredSize(buttonSize);

            rowLayout->addItem(button);
            *nextFreeButton = button;
        }

        mainLayout.addItem(rowLayout);
        mainLayout.setAlignment(rowLayout, sectionModel()->horizontalAlignment());

        // Workaround for QGraphicsLinearLayout bug. The next block can be removed if the issue ever gets fixed.
        // From QGraphicsLinearLayout documentation:
        //     "QGraphicsLinearLayout respects each item's size hints and size policies,
        //     and when the layout contains more space than the items can fill, each item
        //     is arranged according to the layout's alignment for that item."
        // However, the layout ignores the actual width set for the layout and uses only the width
        // taken by the widest item in the layout. We have trouble with this because we have single row
        // KeyButtonAreas which don't take all available width, namely function rows in number/phonenumber.
        // To overcome this we add stretch items to align the row correctly.
        if (numRows == 1) {
            Qt::Alignment alignment = mainLayout.alignment(rowLayout);

            if (alignment & Qt::AlignRight) {
                rowLayout->insertStretch(0);
            } else if (alignment & Qt::AlignCenter) {
                rowLayout->insertStretch(0);
                rowLayout->addStretch();
            }
        }
    } // end foreach row
}

IKeyButton *MButtonArea::keyAt(const QPoint &pos) const
{
    FlickUpButton *result = NULL;
    QPointF scenePos = mapToScene(pos);

    // The drawback of using items() is that we can also get button label or popup.
    // We use "point rect" to obtain the list of items because it allows the use of IntersectsItemBoundingRect
    // and wil not use items' shape() methods. At the time of writing MButton's shape() returns invalid rectangle.
    QList<QGraphicsItem *> itemList = scene()->items(QRectF(scenePos, QSizeF(1, 1)), Qt::IntersectsItemBoundingRect, Qt::DescendingOrder);
    QGraphicsItem *item = itemList.isEmpty() ? 0 : itemList.first();

    if (!item) {
        return result;
    }

    if (dynamic_cast<const QGraphicsItem *>(&popupWidget()) == item) {
        if (itemList.size() < 2) {
            return result;
        }
        // Get the next in Z order. We know such exists.
        item = *(itemList.begin() + 1);
    }

    // For now, if item is not FlickUpButton check if its parentItem is.
    if (!dynamic_cast<FlickUpButton *>(item) && dynamic_cast<FlickUpButton *>(item->parentItem())) {
        item = item->parentItem();
    }

    if ((item != this) && (dynamic_cast<const QGraphicsItem *>(&popupWidget()) != item) && isAncestorOf(item)) {
        result = static_cast<FlickUpButton *>(item);
    }

    return result;
}

void MButtonArea::modifiersChanged(const bool shift, const QChar accent)
{
    foreach (FlickUpButton *button, buttons) {
        button->setModifiers(shift, accent);
    }
}

void MButtonArea::updateButtonGeometries(const int /*availableWidth*/, const int equalButtonWidth)
{
    if (equalButtonWidth < 0 || !layout() || layout()->count() == 0) {
        return;
    }

    for (QVector<FlickUpButton *>::iterator button = buttons.begin(); button != buttons.end(); ++button) {
        (*button)->setPreferredWidth(equalButtonWidth);
    }
    layout()->activate();
}
