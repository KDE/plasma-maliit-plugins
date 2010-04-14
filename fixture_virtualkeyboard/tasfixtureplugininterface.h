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



#ifndef TASFIXTUREPLUGININTERFACE_H
#define TASFIXTUREPLUGININTERFACE_H

#include <QtPlugin>

class QString;
template <typename T, typename Y> class QHash;

class TasFixturePluginInterface
{
public:
     virtual ~TasFixturePluginInterface() {}

     //! Method for calling actions inside plugin.
     virtual bool execute(void *objectInstance,
                          const QString &actionName,
                          const QHash<QString, QString> &parameters,
                          QString &stdOut) = 0;

};

Q_DECLARE_INTERFACE(TasFixturePluginInterface,
        "com.nokia.testability.TasFixturePluginInterface/1.0")

#endif

