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



#ifndef BM_SYMBOLS_H
#define BM_SYMBOLS_H

#include <QtTest/QtTest>
#include <QObject>

class DuiApplication;
class DuiVirtualKeyboardStyleContainer;
class KeyButtonArea;
class KeyboardData;

class Bm_Symbols : public QObject
{
    Q_OBJECT
private:
    DuiApplication *app;
    DuiVirtualKeyboardStyleContainer *style;
    KeyButtonArea *subject;
    KeyboardData *keyboard;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void benchmarkDraw_data();
    void benchmarkDraw();
    void benchmarkLoadXML_data();
    void benchmarkLoadXML();
};

#endif
