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



#ifndef DUIKEYBOARDCOMMON_H
#define DUIKEYBOARDCOMMON_H

/*!
 * \brief State of Copy/Paste button.
 */
enum CopyPasteState {
    //! Copy/Paste button is hidden
    InputMethodNoCopyPaste,

    //! Copy button is accessible
    InputMethodCopy,

    //! Paste button is accessible
    InputMethodPaste
};

/*!
 * Defines Modifier activity state
 */
enum ModifierState {
    ModifierClearState,     //!< Clear state, the modifier is not active
    ModifierLatchedState,   //!< Latched state, the modifier is active until other key is pressed and released
    ModifierLockedState     //!< Locked state, the modifier is active until the same modifier key is pressed and released
};

#endif

