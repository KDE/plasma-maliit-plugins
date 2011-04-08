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
    : MWindow(parent),
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

    averageFPS = log.size() / diff(log[n].start, log[log.size() - 1].end);

    stream << diff(log[n].start, log[n].end) << "; "
           << diff(log[n].end, log[n + 1].start) << "; "
           << averageFPS << "\n";

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

