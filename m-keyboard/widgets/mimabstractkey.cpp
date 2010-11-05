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

#include "mimabstractkey.h"

QList<MImAbstractKey *> MImAbstractKey::mActiveKeys;

MImAbstractKey::~MImAbstractKey()
{
    mActiveKeys.removeAll(this);
}

MImAbstractKey* MImAbstractKey::lastActiveKey()
{
    return (mActiveKeys.isEmpty() ? 0 : mActiveKeys.last());
}

void MImAbstractKey::resetActiveKeys()
{
    while (!mActiveKeys.isEmpty()) {
        MImAbstractKey *key = mActiveKeys.takeFirst();
        key->setSelected(false);
        key->resetTouchPointCount();
    }
}

QList<MImAbstractKey *> MImAbstractKey::filterActiveKeys(bool (predicate)(const MImAbstractKey *))
{
    QList<MImAbstractKey *> result;

    foreach(MImAbstractKey *key, mActiveKeys) {
        if (predicate(key)) {
            result.append(key);
        }
    }

    return result;
}

const QList<MImAbstractKey *> &MImAbstractKey::activeKeys()
{
    return mActiveKeys;
}
