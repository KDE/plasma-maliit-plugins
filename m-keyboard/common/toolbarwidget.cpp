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



#include <QtAlgorithms>
#include "toolbarwidget.h"

ToolbarWidget::ToolbarWidget(WidgetType t)
    : widgetType(t),
      orientation(M::Portrait),
      showOn(Always),
      hideOn(Undefined),
      alignment(Qt::AlignRight),
      visible(false),
      toggle(false),
      pressed(false)
{
}

ToolbarWidget::Action::Action(ToolbarWidget::ActionType t)
    : type(t)
{
}

ToolbarWidget::~ToolbarWidget()
{
    qDeleteAll(actions);
}

ToolbarWidget::WidgetType ToolbarWidget::type() const
{
    return widgetType;
}

QString ToolbarWidget::name() const
{
    return widgetName;
}

bool ToolbarWidget::isVisible() const
{
    return visible;
}

void ToolbarWidget::setVisible(bool v)
{
    visible = v;
}
