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



#ifndef DUIKEYBOARDPLUGIN_H
#define DUIKEYBOARDPLUGIN_H

#include <QObject>

#include "duiinputmethodplugin.h"


/*!
 * \class DuiVirtualKeyboardPlugin
 *
 * \brief Implements plugin for DuiVirtualKeyboard
 */
class DuiKeyboardPlugin: public QObject, public DuiInputMethodPlugin
{
    Q_OBJECT
    Q_INTERFACES(DuiInputMethodPlugin)

public:
    //! \reimp
    virtual QString name() const;
    virtual QStringList languages() const;
    virtual DuiInputMethodBase *createInputMethod(DuiInputContextConnection *icConnection);
    //! \reimp_end
};

#endif
