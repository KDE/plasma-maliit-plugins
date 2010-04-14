/* * This file is part of m-keyboard *
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



#include "ut_layoutsmanager.h"
#include <QDebug>
#include <mgconfitem_stub.h>
#include <keyboarddata.h>
#include <layoutdata.h>
#include <layoutsmanager.h>
#include <memory>


namespace
{
    const QString LanguageListSettingName("/M/InputMethods/Languages");
    const QString NumberFormatSettingName("/M/InputMethods/NumberFormat");
    const QString DisplayLanguageSettingName("/M/i18n/Language");
    const QString LatinNumberFormat("latin");
    const QString ArabicNumberFormat("arabic");
    const QString NumberKeyboardFileArabic("number_ar.xml");
    const QString NumberKeyboardFileLatin("number.xml");
    const QString PhoneNumberKeyboardFileArabic("phonenumber_ar.xml");
    const QString PhoneNumberKeyboardFileLatin("phonenumber.xml");
    const QString PhoneNumberKeyboardFileRussian("phonenumber_ru.xml");
    // Our keyboard loader stub fails for filenames not in this list
    QStringList LoadableKeyboards;
}

// Partial stubbing to control (fake) loading of layout files................

class TestLayoutModel : public LayoutData
{
public:
    TestLayoutModel(const QString &id);

    const QString modelId;
};

TestLayoutModel::TestLayoutModel(const QString &id)
    : modelId(id)
{
}

bool KeyboardData::loadNokiaKeyboard(const QString &fileName)
{
    qDeleteAll(layouts);
    layouts.clear();
    if (LoadableKeyboards.contains(fileName)) {
        layouts.append(new TestLayoutModel(fileName));
        return true;
    }
    return false;
}

const LayoutData *KeyboardData::layout(LayoutData::LayoutType /* type */,
                                       M::Orientation /* orientation */,
                                       bool /* portraitFallback */) const
{
    return layouts.empty() ? NULL : layouts.at(0);
}


// Test init/deinit..........................................................

void Ut_LayoutsManager::initTestCase()
{
    MGConfItem languageListSetting(LanguageListSettingName);

    QStringList langlist;
    langlist << "fi" << "ru" << "ar_SA";
    languageListSetting.set(QVariant(langlist));
}

void Ut_LayoutsManager::cleanupTestCase()
{
}

void Ut_LayoutsManager::init()
{
    LoadableKeyboards.clear();
}

void Ut_LayoutsManager::cleanup()
{
}


// Tests.....................................................................

void Ut_LayoutsManager::testNumberLayouts()
{
    std::auto_ptr<LayoutsManager> subject(new LayoutsManager);
    MGConfItem numberFormatSetting(NumberFormatSettingName);

    // No default when nothing could be loaded
    const TestLayoutModel *layout = dynamic_cast<const TestLayoutModel *>(
                                        subject->layout("fi", LayoutData::Number, M::Landscape));
    QVERIFY(!layout);

    // Latin is used when it can be loaded and when number format setting has no value
    LoadableKeyboards << NumberKeyboardFileLatin;
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi", LayoutData::Number, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileLatin);

    // Also when number format setting is Arabic and Arabic cannot be loaded
    numberFormatSetting.set(QVariant(ArabicNumberFormat));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi", LayoutData::Number, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileLatin);

    // And of course when number format setting is Latin and it can be loaded
    numberFormatSetting.set(QVariant(LatinNumberFormat));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi", LayoutData::Number, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileLatin);

    // And when the number format setting is invalid
    numberFormatSetting.set(QVariant("invalid"));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi", LayoutData::Number, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileLatin);

    // We don't want to use Arabic as fallback if Latin cannot be loaded, even if
    // Arabic can be loaded
    LoadableKeyboards.clear();
    LoadableKeyboards << NumberKeyboardFileArabic;
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi", LayoutData::Number, M::Landscape));
    QVERIFY(!layout);
    // Asking with Arabic language shouldn't make a difference as this
    // stuff works solely based on the number format setting.
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ar_SA", LayoutData::Number, M::Landscape));
    QVERIFY(!layout);

    // If, however, number format is set to Arabic and we can load it,
    // Arabic number layout is used with all languages
    numberFormatSetting.set(QVariant(ArabicNumberFormat));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi", LayoutData::Number, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileArabic);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ar_SA", LayoutData::Number, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileArabic);

    // We can change the setting to Latin on the fly
    LoadableKeyboards << NumberKeyboardFileLatin;
    numberFormatSetting.set(QVariant(LatinNumberFormat));
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi", LayoutData::Number, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileLatin);
}

void Ut_LayoutsManager::testPhoneNumberLayouts()
{
    MGConfItem displayLanguageSetting(DisplayLanguageSettingName);
    displayLanguageSetting.set("en");

    MGConfItem numberFormatSetting(NumberFormatSettingName);
    LoadableKeyboards << NumberKeyboardFileLatin << NumberKeyboardFileArabic;

    // If we can't load any phone number keyboard, we can't have a layout
    numberFormatSetting.set(QVariant(LatinNumberFormat));
    std::auto_ptr<LayoutsManager> subject(new LayoutsManager);
    const TestLayoutModel *layout = dynamic_cast<const TestLayoutModel *>(
                                        subject->layout("fi", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(!layout);

    // If we can load latin phone number keyboard, that's what we get with all
    // display languages (requested layout language doesn't matter),
    // whether our number format is Arabic...
    numberFormatSetting.set(QVariant(ArabicNumberFormat));
    displayLanguageSetting.set("ru");
    LoadableKeyboards << PhoneNumberKeyboardFileLatin;
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ru", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileLatin);
    // ...or latin
    numberFormatSetting.set(QVariant(LatinNumberFormat));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ru", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileLatin);
    displayLanguageSetting.set("fi");
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileLatin);

    qDebug() << "Foo";
    // In normal operation with Latin number format we get...
    LoadableKeyboards << PhoneNumberKeyboardFileArabic << PhoneNumberKeyboardFileRussian;
    subject.reset(new LayoutsManager);
    // ...Russian phone numbers for Russian display language
    displayLanguageSetting.set("ru");
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ru", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileRussian);
    // (... even if we request with different language, i.e. display language
    // setting is all that matters)
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileRussian);
    // ...and Latin for others
    displayLanguageSetting.set("fi");
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileLatin);
    displayLanguageSetting.set("ar");
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ar_SA", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileLatin);

    // But if we switch to Arabic number format, we always get Arabic phone numbers
    numberFormatSetting.set(QVariant(ArabicNumberFormat));
    // That is, for Russian language...
    displayLanguageSetting.set("ru");
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ru", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileArabic);
    // ...and, say, Finnish...
    displayLanguageSetting.set("fi");
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileArabic);
    // ...and of course for Arabic.
    displayLanguageSetting.set("ar");
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ar_SA", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileArabic);
}


QTEST_APPLESS_MAIN(Ut_LayoutsManager);

