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



#include "limitedtimer.h"

#include <QDebug>

namespace
{
    const int MinimalStartInterval = 100; // milliseconds
}

LimitedTimer::LimitedTimer(QObject *parent) : QTimer(parent)
{
    allowedStartTime = QDateTime::currentDateTime().addMSecs(-MinimalStartInterval);
}

void LimitedTimer::start(int msec)
{
    if (QDateTime::currentDateTime() > allowedStartTime
            && msec > 0) {
        QTimer::start(msec);
        allowedStartTime = QDateTime::currentDateTime().addMSecs(MinimalStartInterval);
    }
}

void LimitedTimer::start()
{
    if (QDateTime::currentDateTime() > allowedStartTime) {
        QTimer::start();
        allowedStartTime = QDateTime::currentDateTime().addMSecs(MinimalStartInterval);
    }
}

