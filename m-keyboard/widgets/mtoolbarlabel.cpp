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

#include "mtoolbarlabel.h"
#include "mtoolbarlabelview.h"
#include <mtoolbaritem.h>

MToolbarLabel::MToolbarLabel(QSharedPointer<MToolbarItem> item,
                             QGraphicsItem *parent)
    : MLabel(parent),
      itemPtr(item)
{
    setView(new MToolbarLabelView(this));

    if (item->name().isEmpty()) {
        setObjectName(item->name());
    }

    if (!item->textId().isEmpty()) {
        setText(qtTrId(item->textId().toUtf8().data()));
    } else {
        setText(item->text());
    }
    setVisible(item->isVisible());

    connect(item.data(), SIGNAL(propertyChanged(const QString&)),
            this, SLOT(updateData(const QString&)));
}

MToolbarLabel::~MToolbarLabel()
{
}

QSharedPointer<MToolbarItem> MToolbarLabel::item()
{
    return itemPtr;
}

void MToolbarLabel::updateData(const QString &attribute)
{
    if (attribute == "text") {
        setText(itemPtr->text());
    } else if (attribute == "textId") {
        setText(qtTrId(itemPtr->textId().toUtf8().data()));
    } else if (attribute == "visible") {
        setVisible(itemPtr->isVisible());
        emit availabilityChanged();
    }
}

