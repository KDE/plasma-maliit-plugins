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



#include "widgetbar.h"

#include <MWidget>

#include <QDebug>
#include <QGraphicsLinearLayout>
#include <QPixmap>

// No use for us but prevents MClassFactory bitching about it.
#include <MWidgetCreator>
M_REGISTER_WIDGET_NO_CREATE(WidgetBar)

WidgetBar::WidgetBar(bool useDividers, QGraphicsItem *parent)
    : MStylableWidget(parent),
      mainLayout(*new QGraphicsLinearLayout(Qt::Horizontal, this)),
      useDividers(useDividers)
{
    mainLayout.setSpacing(0); // Spacing is handled by dividers.
    mainLayout.setContentsMargins(0, 0, 0, 0);

    // Update style by calling styleChanged(). This if for widgets that don't have
    // object name set.
    styleChanged();
}

WidgetBar::~WidgetBar()
{
}

int WidgetBar::count() const
{
    return widgets.count();
}

void WidgetBar::insert(int index, MWidget *widget)
{
    Q_ASSERT(widget);

    if (index < 0 || index > count()) {
        qWarning() << "Invalid index given when inserting a widget to WidgetBar.";
        return;
    }

    widgets.insert(index, widget);

    mainLayout.insertItem(index, widget);
    mainLayout.setAlignment(widget, Qt::AlignVCenter); // In case we have widgets that differs in height.
}

void WidgetBar::append(MWidget *widget)
{
    insert(count(), widget);
}

void WidgetBar::remove(MWidget *widget)
{
    mainLayout.removeItem(widget);
    widgets.removeOne(widget);
}

void WidgetBar::clear()
{
    widgets.clear();
    while (mainLayout.count() > 0) {
        mainLayout.removeAt(0);
    }
}

MWidget *WidgetBar::widgetAt(int index) const
{
    MWidget *widget = 0;
    if (index >= 0 && index < count()) {
        widget = widgets.at(index);
    }
    return widget;
}

bool WidgetBar::contains(const MWidget *widget) const
{
    return widgets.contains(const_cast<MWidget *>(widget));
}

int WidgetBar::indexOf(const MWidget *widget) const
{
    return widgets.indexOf(const_cast<MWidget *>(widget));
}

void WidgetBar::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    // Stop propagating
}

void WidgetBar::styleChanged()
{
    setContentsMargins(style()->paddingLeft(), style()->paddingTop(),
                       style()->paddingRight(), 0);
}

