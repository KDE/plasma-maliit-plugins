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

#ifndef MIMKEYVISITOR_H
#define MIMKEYVISITOR_H

#include "mimabstractkey.h"

namespace MImKeyVisitor {
    
    //! \brief Helper class responsible for finding active shift and dead keys.
    class SpecialKeyFinder : public MImAbstractKeyVisitor
    {
    public:
        enum FindMode {
            FindShiftKey,
            FindDeadKey,
            FindBoth
        };

    public:
        explicit SpecialKeyFinder(FindMode newMode = FindBoth);

        MImAbstractKey *shiftKey() const;

        MImAbstractKey *deadKey() const;

        bool operator()(MImAbstractKey *key);

    private:
        MImAbstractKey *mShiftKey;
        MImAbstractKey *mDeadKey;
        FindMode mode;
    };

    //! \brief Helper class for visiting and reseting active keys whilst preserving caps-lock.
    class KeyAreaReset : public MImAbstractKeyVisitor
    {
        public:
            explicit KeyAreaReset();

            bool operator()(MImAbstractKey *key);

            bool hasCapsLocked() const;

        private:
            bool mHasCapsLocked;
    };
}

#endif
