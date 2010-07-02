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



#include "ut_notification.h"
#include <notification.h>
#include "mplainwindow.h"
#include <mvirtualkeyboardstyle.h>

#include <MApplication>
#include <MTheme>

#include <QDebug>
#include <QFile>
#include <QTextStream>

namespace
{
    const int FadeTime = 300;        // Duration of fading in/out animation
    const int HoldTime = 700;        // Time to hold the widget in visible state
}

namespace QTest
{

    template<>
    char *toString(const QFont &font)
    {
        QString string;
        QTextStream stream(&string);

        //print most important fields only
        stream << "QFont("
               << "family = \"" << font.family() << "\", "
               << "pointSize = " << font.pointSize() << ", "
               << "pixelSize = " << font.pixelSize() << ", ...)";

        return qstrdup(string.toLatin1());
    }

}

void Ut_Notification::initTestCase()
{
    static int argc = 1;
    static char *app_name[1] = { (char *) "ut_notification" };

    // Avoid waiting if im server is not responding
    MApplication::setLoadMInputContext(false);
    app = new MApplication(argc, app_name);

    QString cssFile("./test.css");
    if (!QFile::exists(cssFile)) {
        cssFile = "/usr/share/meego-keyboard-tests/ut_notification/test.css";
        QVERIFY(QFile::exists(cssFile));
    }
    QVERIFY(MTheme::instance()->loadCSS(cssFile));

    style = new MVirtualKeyboardStyleContainer;
    style->initialize("MVirtualKeyboard", "MVirtualKeyboardView", 0);

    new MPlainWindow;
}

void Ut_Notification::cleanupTestCase()
{
    delete MPlainWindow::instance();
    delete app;
    app = 0;
}

void Ut_Notification::init()
{
    subject = new Notification(style, 0);
}

void Ut_Notification::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_Notification::testCreate()
{
    QVERIFY(subject != 0);
}

void Ut_Notification::testFadeInFadeOut()
{
    QString text = "Any text string";

    //this test should not crash
    subject->hide();
    subject->displayText(text);
    QVERIFY(subject->isVisible());

    // Fading in
    QTest::qWait(FadeTime);
    QVERIFY(subject->isVisible());

    // Fully visible
    QTest::qWait(HoldTime);
    QVERIFY(subject->isVisible());

    // Fading outs
    QTest::qWait(FadeTime + 100);
    QVERIFY(!subject->isVisible());
}

//This test depends on values in test.css
void Ut_Notification::testCSS()
{
    //construct font in the same way as mstylesheet.cpp
    QFont expected("Nokia Sans Light");
    expected.setPixelSize(42);

    QVERIFY(subject->background == QColor(Qt::red));
    QVERIFY(subject->border == QColor(Qt::green));
    QVERIFY(subject->textColor == QColor(Qt::blue));
    QVERIFY(subject->opacity == 1.0);
    QCOMPARE(subject->font, expected);
}

QTEST_APPLESS_MAIN(Ut_Notification);

