/* * This file is part of meego-keyboard *
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

#include "mtoolbarbutton.h"
#include "mtoolbarbuttonview.h"
#include <mtoolbaritem.h>
#include <MButton>
#include <QFileInfo>
#include <QPixmap>
#include <QDebug>

MToolbarButton::MToolbarButton(QSharedPointer<MToolbarItem> item,
                               QGraphicsItem *parent)
    : MButton(parent),
      icon(0),
      sizePercent(100),
      itemPtr(item)
{
    setView(new MToolbarButtonView(this));

    if (!item->iconId().isEmpty()){
        setIconID(item->iconId());
    } else {
        sizePercent = itemPtr->size();
        setIconFile(itemPtr->icon());
    }

    if (!item->textId().isEmpty()) {
        setText(qtTrId(itemPtr->textId().toUtf8().data()));
    } else {
        setText(itemPtr->text());
    }
    setCheckable(item->toggle());
    if (itemPtr->toggle()) {
        setChecked(itemPtr->pressed());

        connect(this, SIGNAL(clicked(bool)),
                itemPtr.data(), SLOT(setPressed(bool)));
    }
    setVisible(item->isVisible());

    connect(this, SIGNAL(clicked(bool)),
            this, SLOT(onClick()));
    connect(itemPtr.data(), SIGNAL(propertyChanged(const QString&)),
            this, SLOT(updateData(const QString&)));
}

MToolbarButton::~MToolbarButton()
{
    delete icon;
    icon = 0;
}

void MToolbarButton::setIconFile(const QString &newIconFile)
{
    if (iconFile == newIconFile)
        return;

    if (icon) {
        delete icon;
        icon = 0;
        iconFile.clear();
    }

    QFileInfo fileInfo(newIconFile);
    if (fileInfo.exists() && fileInfo.isAbsolute() && fileInfo.isFile()) {
        icon = new QPixmap(newIconFile);
        if (icon) {
            iconFile = newIconFile;
        }
    }
}

void MToolbarButton::setIconPercent(int percent)
{
    if (percent != sizePercent) {
        sizePercent = percent;
    }
}

void MToolbarButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // kludge controller shouldn't really do painting.
    // but this is neccesary now to support drawing the custom icon on the button.
    MButton::paint(painter, option, widget);
    if (iconID().isEmpty() && icon) {
        // TODO: to use style specified in css.
        // scale the icon to limit it insize button according sizePercent and button boundingRect
        // but keep the icon's origin ratio.
        QSizeF size = boundingRect().size();
        QSizeF iconSize = icon->size();
        iconSize.scale(size.width() * sizePercent/100, size.height() * sizePercent/100, Qt::KeepAspectRatio);
        size = (size - iconSize)/2;
        QPointF topLeft = boundingRect().topLeft() + QPointF(size.width(), size.height());
        QRectF iconRect = QRectF(topLeft, iconSize);

        painter->drawPixmap(iconRect, *icon, QRectF(icon->rect()));
    }
}

QSharedPointer<MToolbarItem> MToolbarButton::item()
{
    return itemPtr;
}

void MToolbarButton::updateData(const QString &attribute)
{
    if (attribute == "icon") {
        setIconFile(itemPtr->icon());
    } else if (attribute == "iconId") {
        setIconID(itemPtr->iconId());
    } else if (attribute == "text") {
        setText(itemPtr->text());
    } else if (attribute == "textId") {
        setText(qtTrId(itemPtr->textId().toUtf8().data()));
    } else if (attribute == "pressed" && itemPtr->toggle()) {
        setChecked(itemPtr->pressed());
    } else if (attribute == "visible") {
        setVisible(itemPtr->isVisible());
        emit availabilityChanged();
    } else if (attribute == "size") {
        sizePercent = itemPtr->size();
        update();
    }
}

void MToolbarButton::onClick()
{
    emit clicked(itemPtr.data());
}

