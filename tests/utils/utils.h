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


#ifndef UTILS_H

#include "mimabstractkey.h"

class MPlainWindow;
class MSceneWindow;
class QObject;

// Disable loading of MInputContext and QtMaemo6Style
void disableQtPlugins();

// This prevents some mouse pointer events to be eaten.
void setCustomCompositorRegion(QWidget *window);

// Wait for signal or timeout; use SIGNAL macro for signal
void waitForSignal(const QObject* object, const char* signal, int timeout = 500);

#ifdef MEEGOTOUCH
// Create a scene window, set it to manual managed, and appear it.
MSceneWindow *createMSceneWindow(MPlainWindow *w);
#endif

// copy'n'paste from MImAbstractArea impl file, but counts visits, too:
class SpecialKeyFinder
    : public MImAbstractKeyVisitor
{
public:
    enum FindMode {
        FindShiftKey,
        FindDeadKey,
        FindBoth
    };

private:
    MImAbstractKey *mShiftKey;
    MImAbstractKey *mDeadKey;
    FindMode mode;
    int mVisits;

public:
    explicit SpecialKeyFinder(FindMode newMode = FindBoth)
        : mShiftKey(0)
        , mDeadKey(0)
        , mode(newMode)
        , mVisits(0)
    {}

    MImAbstractKey *shiftKey() const
    {
        return mShiftKey;
    }

    MImAbstractKey *deadKey() const
    {
        return mDeadKey;
    }

    int visits() const
    {
        return mVisits;
    }

    bool operator()(MImAbstractKey *key)
    {
        ++mVisits;

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
};

#endif

