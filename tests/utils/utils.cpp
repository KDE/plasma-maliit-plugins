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


#include "utils.h"
#include <stdlib.h>
#ifdef MEEGOTOUCH
#include <MApplication>
#include <mplainwindow.h>
#include <MSceneWindow>
#include <MSceneManager>
#include <MWindow>
#endif
#include <QObject>
#include <QTimer>
#include <QEventLoop>
#include <QApplication>
#include <QCommonStyle>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

// Disable loading of MInputContext and QtMaemo6Style
void disableQtPlugins()
{
    // prevent loading of minputcontext because we don't need it and starting
    // it might trigger starting of this service by the d-bus.
    if (-1 == unsetenv("QT_IM_MODULE")) {
        qWarning("meego-im-uiserver: unable to unset QT_IM_MODULE.");
    }

#ifdef MEEGOTOUCH
    MApplication::setLoadMInputContext(false);
#endif

    // TODO: Check if hardwiring the QStyle can be removed at a later state.
    QApplication::setStyle(new QCommonStyle);
}

// Wait for signal or timeout; use SIGNAL macro for signal
void waitForSignal(const QObject* object, const char* signal, int timeout)
{
    QEventLoop eventLoop;
    QObject::connect(object, signal, &eventLoop, SLOT(quit()));
    QTimer::singleShot(timeout, &eventLoop, SLOT(quit()));
    eventLoop.exec();
}

// Create a scene window, set it to manual managed, and appear it.
#ifdef MEEGOTOUCH
MSceneWindow * createMSceneWindow(MWindow *w)
{
    MSceneWindow *sceneWindow = new MSceneWindow;
    sceneWindow->setManagedManually(true); // we want the scene window to remain in origin
    w->sceneManager()->appearSceneWindowNow(sceneWindow);
    w->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    return sceneWindow;
}
#endif

