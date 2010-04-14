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

#include "duiimcorrectioncandidatewindow.h"
#include <QDebug>
#include <QString>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

DuiImCorrectionCandidateWindow::DuiImCorrectionCandidateWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("DuiImCorrectionCandidateWindow");
    setAttribute(Qt::WA_TranslucentBackground);
    Display *dpy =  QX11Info::display();
    const Qt::WindowFlags windowFlags = Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;
    setWindowFlags(windowFlags);

    // Hint to the window manager that we don't want input focus
    // for the inputmethod window
    XWMHints wmhints;
    wmhints.flags = InputHint;
    wmhints.input = False;
    XSetWMHints(dpy, winId(), &wmhints);
    Atom input = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_INPUT", False);
    XChangeProperty(dpy, winId(), XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False), XA_ATOM, 32,
                    PropModeReplace, (unsigned char *) &input, 1);

    setFocusPolicy(Qt::NoFocus);
}

void DuiImCorrectionCandidateWindow::handleRegionUpdate(const QRegion &region)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << region;
    // selective compositing
    if (isVisible() && region.isEmpty()) {
        hide();
    } else if (!isVisible() && !region.isEmpty()) {
        qDebug() << "show  DuiImCorrectionCandidateWindow!";
        show();
        raise();
    }
}

