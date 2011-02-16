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
class MImAbstractKeyArea;
class KeyboardData;
class MSceneWindow;
class LoggingWindow;
class MImAbstractKey;
class MImKeyArea;
class QWidget;

class Bm_Painting : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MSceneWindow *sceneWindow;
    LoggingWindow *window;
    QWidget *widget;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void benchmarkPaintDuringKeyPresses_data();
    void benchmarkPaintDuringKeyPresses();

    void benchmarkPaintDuringHorizontalLayoutChange_data();
    void benchmarkPaintDuringHorizontalLayoutChange();

private:
    void commonDataSetup(const QString& testcaseName);
    void commonWindowSetup(bool hardwareRendering, bool compositing);
    QSize defaultLayoutSize(MImKeyArea *keyArea);
    MImAbstractKey *keyAt(MImKeyArea *keyArea, unsigned int row, unsigned int column) const;
};

#endif
