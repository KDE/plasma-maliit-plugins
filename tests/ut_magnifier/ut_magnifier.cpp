/* * This file is part of dui-vkb-magnifier *
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



#include "ut_magnifier.h"
#include <magnifier.h>
#include <MApplication>
#include <MTheme>

#include <QFile>
#include <QTextStream>

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

void Ut_Magnifier::initTestCase()
{
    static int argc = 1;
    static char *app_name[1] = { (char *) "ut_magnifier" };

    // Avoid waiting if im server is not responding
    MApplication::setLoadMInputContext(false);
    app = new MApplication(argc, app_name);

    MTheme::instance()->loadCSS(":test.css");
}

void Ut_Magnifier::cleanupTestCase()
{
    delete app;
    app = 0;
}

void Ut_Magnifier::init()
{
    subject = new Magnifier(0);
}

void Ut_Magnifier::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_Magnifier::testCreate()
{
    QVERIFY(subject != 0);
}

void Ut_Magnifier::testOther()
{
    QPointF pos(100, 20);

    QVERIFY(subject->pos() != pos);
    subject->setPos(pos);
    QVERIFY(subject->pos() == pos);
}

QTEST_APPLESS_MAIN(Ut_Magnifier);

