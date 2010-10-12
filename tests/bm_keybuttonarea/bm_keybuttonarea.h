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



#ifndef BM_LAYOUT_H
#define BM_LAYOUT_H

#include <QtTest/QtTest>
#include <QObject>

class MApplication;
class KeyButtonArea;
class KeyboardData;

class Bm_KeyButtonArea : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    KeyButtonArea *subject;
    KeyboardData *keyboard;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void benchmarkPreDraw_data();
    void benchmarkPreDraw();
    void benchmarkLoadXML_data();
    void benchmarkLoadXML();
    void benchmarkPaint_data();
    void benchmarkPaint();
};

#endif
