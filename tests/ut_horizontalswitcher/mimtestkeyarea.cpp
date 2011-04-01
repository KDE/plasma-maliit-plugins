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



#include "mimtestkeyarea.h"
#include "mimabstractkeyarea_p.h"

MImTestKeyArea::MImTestKeyArea(const LayoutData::SharedLayoutSection &section,
               bool usePopup,
               QGraphicsWidget *parent)
    : MImAbstractKeyArea(new MImAbstractKeyAreaPrivate(section, this), usePopup, parent),
      setKeyOverridesCalls(0)
{
}

MImTestKeyArea::~MImTestKeyArea()
{
}

QList<const MImAbstractKey *> MImTestKeyArea::keys() const
{
    return QList<const MImAbstractKey *>();
}

MImAbstractKey * MImTestKeyArea::findKey(const QString &)
{
    return 0;
}

MImAbstractKey * MImTestKeyArea::keyAt(const QPoint &) const
{
    return 0;
}

void MImTestKeyArea::updateKeyGeometries(int)
{
}

void MImTestKeyArea::setContentType(M::TextContentType)
{
}

bool MImTestKeyArea::allowedHorizontalFlick() const
{
    return d_ptr->allowedHorizontalFlick;
}

void MImTestKeyArea::setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides)
{
    ++setKeyOverridesCalls;
    setKeyOverridesParam = overrides;
}

void MImTestKeyArea::setToggleKeyState(bool)
{
}

void MImTestKeyArea::setComposeKeyState(bool)
{
}

void MImTestKeyArea::resetActiveKeys()
{
}
