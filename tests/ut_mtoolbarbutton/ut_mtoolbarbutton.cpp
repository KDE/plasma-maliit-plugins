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



#include "ut_mtoolbarbutton.h"
#include "mtoolbarbutton.h"
#include "mapplication.h"
#include "utils.h"
#include <mtoolbardata.h>
#include <mtoolbarlayout.h>
#include "mgconfitem_stub.h"

#include <QFile>


namespace
{
    const QString TargetSettingsName("/meegotouch/target/name");
    const QString DefaultTargetName("Default");

    QString ToolbarFileName  = "/testtoolbar.xml";

    QSharedPointer<MToolbarItem> findItem(QSharedPointer<MToolbarData> toolbar, const QString &name)
    {
        if (!toolbar)
            return QSharedPointer<MToolbarItem>();

        foreach(const QSharedPointer<MToolbarItem> &item, toolbar->items()) {
            if (item->name() == name)
                return item;
        }

        return QSharedPointer<MToolbarItem>();
    }

}


// Stubbing..................................................................

void MToolbarButton::setIconFile(const QString &newIconFile)
{
    //reimplement to avoid checking the exist of icon file.
    iconFile = newIconFile;
}

void Ut_MToolbarButton::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_mtoolbarbutton",
                                  (char *) "-software" };
    // this value is required by the theme daemon
    MGConfItem(TargetSettingsName).set(DefaultTargetName);

    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);

    ToolbarFileName = QCoreApplication::applicationDirPath() + ToolbarFileName;
    QVERIFY(QFile::exists(ToolbarFileName));
}

void Ut_MToolbarButton::init()
{
    //fill up toolbar with some data
    toolbarData = QSharedPointer<MToolbarData>(new MToolbarData);
    bool ok = toolbarData->loadToolbarXml(ToolbarFileName);
    QVERIFY(ok);
}


void Ut_MToolbarButton::cleanup()
{
    subject.clear();
}

void Ut_MToolbarButton::cleanupTestCase()
{
    toolbarData.clear();
    delete app;
    app = 0;
}

void Ut_MToolbarButton::testStylingName_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("styleName");

    QTest::newRow("text") << "text" << "MToolbarTextButton";
    QTest::newRow("icon") << "icon" << "MToolbarIconButton";
    QTest::newRow("text highlighted") << "textHighlighted" << "MToolbarTextButtonHighlighted";
    QTest::newRow("icon highlighted") << "iconHighlighted" << "MToolbarIconButtonHighlighted";
}

void Ut_MToolbarButton::testStylingName()
{
    QFETCH(QString, name);
    QFETCH(QString, styleName);

    QSharedPointer<MToolbarItem> item = findItem(toolbarData, name);
    QVERIFY(item);

    subject = QSharedPointer<MToolbarButton>(new MToolbarButton(item));
    QCOMPARE(subject->styleName(), styleName);
}

QTEST_APPLESS_MAIN(Ut_MToolbarButton);

