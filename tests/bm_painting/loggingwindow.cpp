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


#include "loggingwindow.h"

#include <QFile>
#include <QTextStream>
#include <QEvent>
#include <QDebug>

static qreal diff(const timeval &x, const timeval &y)
{
    timeval difference;

    difference.tv_usec = y.tv_usec - x.tv_usec;
    difference.tv_sec = y.tv_sec;
    if (difference.tv_usec < 0) {
        difference.tv_usec += 1000000;
        difference.tv_sec -= 1;
    }
    difference.tv_sec -= x.tv_sec;

    return qreal(difference.tv_sec) + difference.tv_usec / qreal(1000000);
}

static timeval InvalidTime = { 0, 0 };

inline bool operator==(const timeval &x, const timeval &y)
{
    return x.tv_sec == y.tv_sec && x.tv_usec == y.tv_usec;
}
inline bool operator!=(const timeval &x, const timeval &y)
{
    return !(x == y);
}

LoggingWindow::LoggingWindow(QWidget *parent)
    : MPlainWindow(parent),
      loggingEnabled(false)
{
    log.reserve(10000);
}

bool LoggingWindow::viewportEvent(QEvent *event)
{
    bool result;
    EventTimestamp time;

    gettimeofday(&time.start, 0);
    result = MWindow::viewportEvent(event);
    gettimeofday(&time.end, 0);

    if (loggingEnabled && event->type() == QEvent::Paint)
        log.append(time);

    return result;
}

bool LoggingWindow::writeResults(const QString &fileName)
{
    QFile file(fileName);

    if (log.size() < 2)
        return false;

    if (!file.open(QIODevice::WriteOnly))
        return false;

    QTextStream stream(&file);

    stream << "painting duration; delay; FPS\n";

    int n = 0;

    while ((n < log.size() - 2) && (log[n].start == InvalidTime))
        ++n;

    stream << diff(log[n].start, log[n].end) << "; "
           << diff(log[n].end, log[n + 1].start) << "; "
           << log.size() / diff(log[n].start, log[log.size() - 1].end) << "\n";

    for (++n; n < log.size() - 2; ++n) {
        if (log[n].start != InvalidTime) {
            stream << diff(log[n].start, log[n].end) << "; ";
            if (log[n + 1].start != InvalidTime) {
                stream << diff(log[n].end, log[n + 1].start);
            }
        }
        stream << "\n";
    }

    return true;
}

void LoggingWindow::logMark()
{
    EventTimestamp mark = { InvalidTime, InvalidTime };
    log.append(mark);
}

