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



#include <mgconfitem_stub.h>
#include "hwkbcharloopsmanager.h"
#include "hwkbcharloops.h"
#include "ut_hwkbcharloopsmanager.h"
#include <QDebug>
#include <memory>


namespace
{
    const QString SystemDisplayLanguage("/meegotouch/i18n/language");
}

bool HwKbCharLoopsManager::loadCharLoops(const QString &fileName)
{
    Q_UNUSED(fileName);
    qDeleteAll(charLoops);
    charLoops.clear();

    charLoops.insert("en_gb", new HwKbCharacterLoops("en_gb", ""));
    charLoops.insert("fi", new HwKbCharacterLoops("fi", ""));
    charLoops.insert("ar", new HwKbCharacterLoops("ar", ""));
    return true;
}

void Ut_HwKbCharLoopsManager::initTestCase()
{
    m_subject = new HwKbCharLoopsManager;
}

void Ut_HwKbCharLoopsManager::cleanupTestCase()
{
    delete m_subject;
    m_subject = 0;
}

void Ut_HwKbCharLoopsManager::init()
{
    QVERIFY(m_subject->setCharLoopsLanguage("en_gb"));
}

void Ut_HwKbCharLoopsManager::cleanup()
{
}

void Ut_HwKbCharLoopsManager::testLoadLanguage()
{
    QVERIFY(m_subject->setCharLoopsLanguage("en_gb"));
    QCOMPARE(m_subject->currentCharLoopLanguage, QString("en_gb"));
    QVERIFY(m_subject->setCharLoopsLanguage("fi"));
    QCOMPARE(m_subject->currentCharLoopLanguage, QString("fi"));
    QVERIFY(m_subject->setCharLoopsLanguage("ar"));
    QCOMPARE(m_subject->currentCharLoopLanguage, QString("ar"));
}

void Ut_HwKbCharLoopsManager::testSyncLanguage()
{
    QCOMPARE(m_subject->currentCharLoopLanguage, QString("en_gb"));
    MGConfItem systemDisplayLanguage(SystemDisplayLanguage);
    systemDisplayLanguage.set(QVariant("fi"));
    QCOMPARE(m_subject->currentCharLoopLanguage, QString("fi"));
    systemDisplayLanguage.set(QVariant("ar"));
    QCOMPARE(m_subject->currentCharLoopLanguage, QString("ar"));
}

QTEST_APPLESS_MAIN(Ut_HwKbCharLoopsManager);

