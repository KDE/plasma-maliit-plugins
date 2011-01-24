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

#include "mimkeyvisitor.h"
#include "mimabstractkeyarea.h"

namespace MImKeyVisitor {
    SpecialKeyFinder::SpecialKeyFinder(FindMode newMode)
        : MImAbstractKeyVisitor()
        , mShiftKey(0)
        , mDeadKey(0)
        , mode(newMode)
        {
        }

    MImAbstractKey *SpecialKeyFinder::shiftKey() const
    {
        return mShiftKey;
    }

    MImAbstractKey *SpecialKeyFinder::deadKey() const
    {
        return mDeadKey;
    }

    bool SpecialKeyFinder::operator()(MImAbstractKey *key)
    {
        if (!key) {
            return false;
        }

        if (key->isShiftKey()) {
            mShiftKey = key;
        } else if (key->isDeadKey()) {
            mDeadKey = key;
        }

        switch (mode) {
            case FindShiftKey:
                if (mShiftKey) {
                    return true;
                }
                break;

            case FindDeadKey:
                if (mDeadKey) {
                    return true;
                }
                break;

            case FindBoth:
                if (mShiftKey && mDeadKey) {
                    return true;
                }
                break;
        }

        return false;
    }

    MainAreaReset::MainAreaReset()
        : mHasCapsLocked(false)
        {
        }

    bool MainAreaReset::operator()(MImAbstractKey *key)
    {
        if (!key) {
            return false;
        }
        if (key->isShiftKey()
                && (key->state() == MImAbstractKey::Selected)) {
            // OK, in caps-locked mode. But don't reset shift key:
            mHasCapsLocked = true;
        } else {
            key->resetTouchPointCount();
        }

        return false;
    }

    bool MainAreaReset::hasCapsLocked() const
    {
        return mHasCapsLocked;
    }
}
