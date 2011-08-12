/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list 
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list 
 * of conditions and the following disclaimer in the documentation and/or other materials 
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be 
 * used to endorse or promote products derived from this software without specific 
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
    subject = new Notification(0);
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
    QString text;
    QRectF rect(0.0, 0.0, 1000.0, 100.0);

    // Empty message -> no crash should happen
    subject->setMessageAndGeometry("", QRectF());
    QVERIFY(subject->message == "");

    text = "This is a longer text string";
    subject->setMessageAndGeometry(text, rect);
    QCOMPARE(subject->message, text);
}

void Ut_Notification::testFadeInFadeOut()
{
    QString text = "Any text string";

    //this test should not crash
    subject->hide();
    subject->displayText(text, QRectF());
    QVERIFY(subject->isVisible());

    // Fading in
    QTest::qWait(subject->style()->fadeTime());
    QVERIFY(subject->isVisible());

    // Fully visible
    QTest::qWait(subject->style()->holdTime());
    QVERIFY(subject->isVisible());

    // Fading outs
    QTest::qWait(subject->style()->fadeTime() + 100);
    QVERIFY(!subject->isVisible());
}

QTEST_APPLESS_MAIN(Ut_Notification);

