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

#include "simplefilelog.h"

#include <QDir>
#include <QCoreApplication>

namespace {
    const char *const MImUserDirectory = ".meego-im";
}

SimpleFileLog::SimpleFileLog(const QString &fileName)
    : mFile()
    ,  mStream(&mFile)
{
    QString logFilePath = QString("%1/%2/%3-%4").arg(QDir::homePath())
                          .arg(MImUserDirectory)
                          .arg(QCoreApplication::applicationPid())
                          .arg(fileName);
    mFile.setFileName(logFilePath);

    mStream.setCodec("utf-8");

    if (!QDir::home().exists(MImUserDirectory)) {
        QDir::home().mkdir(MImUserDirectory);
    }
    mFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
}

QTextStream & SimpleFileLog::stream()
{
    return mStream;
}

void SimpleFileLog::flush()
{
    mStream << "\n";
    mStream.flush();
}
