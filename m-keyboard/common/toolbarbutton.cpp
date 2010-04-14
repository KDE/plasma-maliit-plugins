/* * This file is part of dui-keyboard *
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
#include "toolbarbutton.h"

ToolbarButton::ToolbarButton()
    : orientation(Dui::Portrait),
      showOn(Always),
      hideOn(Undefined),
      alignment(Qt::AlignRight),
      visible(false)
{
}

ToolbarButton::~ToolbarButton()
{
    qDeleteAll(actions);
}

ToolbarButton::Action::Action(ToolbarButton::ActionType t)
    : type(t)
{
}

bool ToolbarButton::isVisible() const
{
    return visible;
}

void ToolbarButton::setVisible(bool v)
{
    visible = v;
}

