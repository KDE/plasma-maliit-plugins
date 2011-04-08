/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list 
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list 
 * of conditions and the following disclaimer in the documentation and/or other materials 
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be 
 * used to endorse or promote products derived from this software without specific 
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */


#include "utils.h"
#include <stdlib.h>
#ifdef HAVE_MEEGOTOUCH
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

#ifdef HAVE_MEEGOTOUCH
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
#ifdef HAVE_MEEGOTOUCH
MSceneWindow * createMSceneWindow(MWindow *w)
{
    MSceneWindow *sceneWindow = new MSceneWindow;
    sceneWindow->setManagedManually(true); // we want the scene window to remain in origin
    w->sceneManager()->appearSceneWindowNow(sceneWindow);
    w->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    return sceneWindow;
}
#endif

