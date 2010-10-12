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



#include "bm_keybuttonarea.h"
#include "singlewidgetbuttonarea.h"
#include "keyboarddata.h"
#include "utils.h"

#include <MApplication>
#include <MTheme>

#include <QDir>


void Bm_KeyButtonArea::initTestCase()
{
    static int argc = 2;
    static char *app_name[2] = { (char *) "bm_keybuttonarea",
                                 (char *) "-local-theme" };

    disableQtPlugins();
    app = new MApplication(argc, app_name);
}

void Bm_KeyButtonArea::cleanupTestCase()
{
    delete app;
    app = 0;
}

void Bm_KeyButtonArea::init()
{
    keyboard = 0;
    subject = 0;
}

void Bm_KeyButtonArea::cleanup()
{
    delete subject;
    subject = 0;
    delete keyboard;
    keyboard = 0;
}

void Bm_KeyButtonArea::benchmarkPreDraw_data()
{
    QDir dir("/usr/share/meegotouch/virtual-keyboard/layouts/");
    QStringList filters;
    QFileInfoList files;
    QFileInfo info;

    QTest::addColumn<QString>("filename");
    filters << "??.xml";
    files = dir.entryInfoList(filters);
    for (int n = files.count() - 1; n >= 0; --n) {
        info = files.at(n);
        QTest::newRow(info.fileName().toLatin1().constData()) << info.fileName();
    }
    for (int n = files.count() - 1; n >= 0; --n) {
        info = files.at(n);
        QTest::newRow(info.fileName().toLatin1().constData()) << info.fileName();
    }
}

void Bm_KeyButtonArea::benchmarkPreDraw()
{
    QFETCH(QString, filename);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(filename));
    subject = new SingleWidgetButtonArea(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection));

    QBENCHMARK {
        subject->updateButtonGeometriesForWidth(864);
        subject->updateButtonModifiers();
    }
}

void Bm_KeyButtonArea::benchmarkLoadXML_data()
{
    QDir dir("/usr/share/meegotouch/virtual-keyboard/layouts/");
    QStringList filters;
    QFileInfoList files;
    QFileInfo info;

    QTest::addColumn<QString>("filename");
    filters << "??.xml";
    files = dir.entryInfoList(filters);
    for (int n = files.count() - 1; n >= 0; --n) {
        info = files.at(n);
        QTest::newRow(info.fileName().toLatin1().constData()) << info.fileName();
    }
}

// TODO: This test should not be here anymore.
//       Maybe test initialization speed of widget with section data model?
void Bm_KeyButtonArea::benchmarkLoadXML()
{
    QFETCH(QString, filename);

    QBENCHMARK {
        for (int n = 0; n < 5; ++n) {
            keyboard = new KeyboardData;
            QVERIFY(keyboard->loadNokiaKeyboard(filename));
            delete keyboard;
            keyboard = 0;
        }
    }
}

void Bm_KeyButtonArea::benchmarkPaint_data()
{
    QDir dir("/usr/share/meegotouch/virtual-keyboard/layouts/");
    QStringList filters;
    QFileInfoList files;
    QFileInfo info;

    QTest::addColumn<QString>("filename");
    filters << "??.xml";
    files = dir.entryInfoList(filters);
    for (int n = files.count() - 1; n >= 0; --n) {
        info = files.at(n);
        QTest::newRow(info.fileName().toLatin1().constData()) << info.fileName();
    }
    for (int n = files.count() - 1; n >= 0; --n) {
        info = files.at(n);
        QTest::newRow(info.fileName().toLatin1().constData()) << info.fileName();
    }
}

void Bm_KeyButtonArea::benchmarkPaint()
{
    QImage *image = new QImage(QSize(864, 480), QImage::Format_ARGB32_Premultiplied);
    QPainter painter;

    QFETCH(QString, filename);

    QVERIFY(painter.begin(image) == true);
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(filename));
    subject = new SingleWidgetButtonArea(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection));

    QBENCHMARK {
        subject->paint(&painter, 0 , 0);
    }

    painter.end();
    delete image;
}

QTEST_APPLESS_MAIN(Bm_KeyButtonArea);
