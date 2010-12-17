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


#ifndef LOGGINGWINDOW_H
#define LOGGINGWINDOW_H

// this test could not be compiled for Windows
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <mplainwindow.h>
#include <QVector>
#include <QString>

class LoggingWindow : public MPlainWindow
{
public:
    LoggingWindow(QWidget *parent);

    bool writeResults(const QString &);

    bool loggingEnabled;

    void logMark();

protected:
    virtual bool viewportEvent(QEvent *event);

private:
    struct EventTimestamp {
        timeval start;
        timeval end;
    };
    QVector<EventTimestamp> log;
};

#endif // LOGGINGWINDOW_H
