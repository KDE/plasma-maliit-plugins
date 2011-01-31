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
#include "utils.h"
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
    static int argc = 2;
    static char *app_name[2] = { (char *) "ut_notification",
                                 (char *) "-software" };

    disableQtPlugins();
    app = new MApplication(argc, app_name);

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

void Ut_Notification::testSetMessageAndGeometry()
{
    // Empty message -> no crash should happen
    subject->setMessageAndGeometry("", QRectF());
    QVERIFY(subject->message == "");
    // One word with no available width-> no crash should happen
    subject->setMessageAndGeometry("One", QRectF());
    QVERIFY(subject->message == "One");
    // One word with enough width -> no crash should happen
    subject->setMessageAndGeometry("One", QRectF(0.0, 0.0, 1000.0, 0.0));
    QVERIFY(subject->message == "One");
    // A longer text with no available width -> every word should be a new line
    subject->setMessageAndGeometry("This is a longer text string", QRectF());
    QVERIFY(subject->message == "This\nis\na\nlonger\ntext\nstring");
    // A longer text with available width -> no line breaks
    subject->setMessageAndGeometry("This is a longer text string",
                                   QRectF(0.0, 0.0, 1000.0, 0.0));
    QVERIFY(subject->message == "This is a longer text string");
    // An other example text with available width -> some line breaks
    subject->setMessageAndGeometry("Verylonglongword and some shorter",
                                   QRectF(0.0, 0.0, 100.0, 0.0));
    QVERIFY(subject->message == "Verylonglongword\nand\nsome\nshorter");
}

void Ut_Notification::testFadeInFadeOut()
{
    QString text = "Any text string";

    //this test should not crash
    subject->hide();
    subject->displayText(text, QRectF());
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

QTEST_APPLESS_MAIN(Ut_Notification);

