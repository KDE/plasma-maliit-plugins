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

#ifndef SIMPLEFILELOG_H
#define SIMPLEFILELOG_H

#include <QFile>
#include <QTextStream>

//! \internal
class SimpleFileLog
{
public:
    //! \brief Constructor. File will be opened under the meego-im home directory,
    //! and be prefixed with the current PID. Ex: /home/user/.meego-im/4912-file.cvs
    explicit SimpleFileLog(const QString &fileName);

    //! \brief The stream to log to.
    QTextStream & stream();

    //! \brief Insert a marker (\n), and flush the textstream to file.
    void flush();

private:
    QFile mFile;
    QTextStream mStream;
};

#endif // SIMPLEFILELOG_H
