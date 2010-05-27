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



#ifndef MKEYBOARDPLUGIN_H
#define MKEYBOARDPLUGIN_H

#include <QObject>

#include "minputmethodplugin.h"


/*!
 * \class MVirtualKeyboardPlugin
 *
 * \brief Implements plugin for MVirtualKeyboard
 */
class MKeyboardPlugin: public QObject, public MInputMethodPlugin
{
    Q_OBJECT
    Q_INTERFACES(MInputMethodPlugin)

public:
    //! \reimp
    virtual QString name() const;
    virtual QStringList languages() const;
    virtual MInputMethodBase *createInputMethod(MInputContextConnection *icConnection);
    virtual MInputMethodSettingsBase *createInputMethodSettings();
    virtual QSet<MIMHandlerState> supportedStates() const;
    //! \reimp_end
};

#endif
