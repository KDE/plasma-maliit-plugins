/* * This file is part of m-keyboard *
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


#ifndef MKEYBOARDHOST_P_H
#define MKEYBOARDHOST_P_H

#include <QTimer>
#include "mkeyboardhost.h"

class MKeyboardHost::CycleKeyHandler : public QObject
{
    Q_OBJECT
public:
    explicit CycleKeyHandler(MKeyboardHost &parent);
    virtual ~CycleKeyHandler();
    bool handleTextInputKeyClick(const KeyEvent &event);
    void reset();

private slots:
    void commitCycleKey();

private:
    MKeyboardHost &host;    // The MKeyboardHost to use
    QTimer timer;           // Timer to kick commit
    int cycleIndex;         // Index to the cycle string
    QString cycleText;
    KeyEvent prevEvent;
};

#endif
