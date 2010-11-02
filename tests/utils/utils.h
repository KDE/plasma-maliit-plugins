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

class MPlainWindow;
class QObject;

// Disable loading of MInputContext and QtMaemo6Style
void disableQtPlugins();

// Wait for signal or timeout; use SIGNAL macro for signal
void waitForSignal(const QObject* object, const char* signal, int timeout = 500);

#ifdef MEEGOTOUCH
// Create graphics scene
void createMScene(MPlainWindow *w);
#endif

#endif

