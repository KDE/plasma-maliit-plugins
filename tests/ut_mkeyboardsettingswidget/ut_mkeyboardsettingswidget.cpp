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

#include "ut_mkeyboardsettingswidget.h"
#include <mgconfitem_stub.h>
#include <mkeyboardsettingswidget.h>
#include "utils.h"

#include <MApplication>
#include <MSceneManager>
#include <MList>
#include <MButton>
#include <MContentItem>
#include <MDialog>
#include <MLabel>

#include <QGraphicsLayout>
#include <QModelIndex>
#include <QSignalSpy>

namespace {
    const QString SettingsImCorrection("/meegotouch/inputmethods/virtualkeyboard/correctionenabled");
    const QString SettingsImCorrectionSpace("/meegotouch/inputmethods/virtualkeyboard/correctwithspace");
};

// Stubbing..................................................................
int MSceneManager::execDialog(MDialog *dialog)
{   
    // avoid MSceneManager create private even loop
    appearSceneWindowNow(dialog);
    return 0;
}

void Ut_MKeyboardSettingsWidget::initTestCase()
{
    static char *argv[2] = { (char *) "ut_mkeyboardsettingswidget",
                             (char *) "-software" };
    static int argc = 2;
    disableQtPlugins();
    app = new MApplication(argc, argv);
    settingsObject = new MKeyboardSettings();
}

void Ut_MKeyboardSettingsWidget::cleanupTestCase()
{
    delete settingsObject;
    delete app;
}

void Ut_MKeyboardSettingsWidget::init()
{
    subject = new MKeyboardSettingsWidget(settingsObject);
    QVERIFY(subject->selectedKeyboardsItem);
    QVERIFY(subject->errorCorrectionSwitch);
    QVERIFY(subject->correctionSpaceSwitch);
}

void Ut_MKeyboardSettingsWidget::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_MKeyboardSettingsWidget::testShowKeyboardList()
{
    subject->selectedKeyboardsItem->click();
    QVERIFY(subject->keyboardDialog);
    QVERIFY(subject->keyboardDialog->isVisible());
    QVERIFY(subject->keyboardList);

    QVERIFY(subject->keyboardList->itemModel());
    // available keyboard list should display all available keyboards.
    QMap<QString, QString> availableKeyboards = settingsObject->availableKeyboards();
    QCOMPARE(subject->keyboardList->itemModel()->rowCount(), availableKeyboards.count());
    QStringList availableKeyboardTitles = availableKeyboards.values();
    for (int column = 0; column < subject->keyboardList->itemModel()->columnCount(); column++) {
        for (int row = 0; row < subject->keyboardList->itemModel()->rowCount(); row++) {
            const QModelIndex mIndex = subject->keyboardList->itemModel()->index(row, column);
            QVERIFY(availableKeyboardTitles.contains(subject->keyboardList->itemModel()->data(mIndex).toString()));
        }
    }

    QVERIFY(subject->keyboardList->selectionModel());
    // selected keyboards should be the same as keyboards.
    QMap<QString, QString> selectedKeyboards = settingsObject->selectedKeyboards();
    QCOMPARE(subject->keyboardList->selectionModel()->selectedRows().count(),
             selectedKeyboards.count() );
    QStringList selectedKeyboardTitles = selectedKeyboards.values();
    foreach (const QModelIndex &mIndex, subject->keyboardList->selectionModel()->selectedRows()) {
        QVERIFY(selectedKeyboardTitles.contains(subject->keyboardList->itemModel()->data(mIndex).toString()));
    }

    bool firstItemIsSelected = subject->keyboardList->selectionModel()->selectedRows().contains(subject->keyboardList->itemModel()->index(0,0));
    subject->keyboardList->selectItem(subject->keyboardList->itemModel()->index(0,0));
    QMap<QString, QString> newSelectedKeyboards = settingsObject->selectedKeyboards();
    if (firstItemIsSelected) {
        QCOMPARE(newSelectedKeyboards.count(), selectedKeyboards.count() - 1);
    } else {
        QCOMPARE(newSelectedKeyboards.count(), selectedKeyboards.count() + 1);
    }
}

void Ut_MKeyboardSettingsWidget::testKeyboardCorrectionSettings()
{
    MGConfItem errorCorrectionSetting(SettingsImCorrection);
    MGConfItem correctionSpaceSetting(SettingsImCorrectionSpace);

    // Check the error correction setting alone
    settingsObject->setErrorCorrection(true);
    QCOMPARE(subject->errorCorrectionSwitch->isChecked(), true);
    settingsObject->setErrorCorrection(false);
    QCOMPARE(subject->errorCorrectionSwitch->isChecked(), false);

    // Check the "select with space" setting alone
    // Note: Error correction must be enabled
    settingsObject->setErrorCorrection(true);
    settingsObject->setCorrectionSpace(true);
    QCOMPARE(subject->correctionSpaceSwitch->isChecked(), true);
    settingsObject->setCorrectionSpace(false);
    QCOMPARE(subject->correctionSpaceSwitch->isChecked(), false);
    // Check if switching the error correction off disables the "select with space"
    settingsObject->setErrorCorrection(true);
    settingsObject->setCorrectionSpace(true);
    QCOMPARE(subject->correctionSpaceSwitch->isChecked(), true);
    settingsObject->setErrorCorrection(false);
    QCOMPARE(subject->correctionSpaceSwitch->isEnabled(), false);
    QCOMPARE(subject->correctionSpaceSwitch->isChecked(), false);

    // Check the behaviour of the error correction toggle button alone
    QSignalSpy spyErrorCorrection(subject->errorCorrectionSwitch, SIGNAL(toggled(bool)));
    subject->setErrorCorrectionState(true);
    QCOMPARE(spyErrorCorrection.count(), 1);
    QCOMPARE(spyErrorCorrection.takeFirst().at(0).toBool(), true);
    QCOMPARE(errorCorrectionSetting.value().toBool(), true);

    spyErrorCorrection.clear();
    subject->setErrorCorrectionState(false);
    QCOMPARE(spyErrorCorrection.count(), 1);
    QCOMPARE(spyErrorCorrection.takeFirst().at(0).toBool(), false);
    QCOMPARE(errorCorrectionSetting.value().toBool(), false);

    // Check the behaviour of the error correction toggle button alone
    QSignalSpy spyCorrectionSpace(subject->correctionSpaceSwitch, SIGNAL(toggled(bool)));
    subject->setCorrectionSpaceState(true);
    QCOMPARE(spyCorrectionSpace.count(), 1);
    QCOMPARE(spyCorrectionSpace.takeFirst().at(0).toBool(), true);
    QCOMPARE(correctionSpaceSetting.value().toBool(), true);

    spyCorrectionSpace.clear();
    subject->setCorrectionSpaceState(false);
    QCOMPARE(spyCorrectionSpace.count(), 1);
    QCOMPARE(spyCorrectionSpace.takeFirst().at(0).toBool(), false);
    QCOMPARE(correctionSpaceSetting.value().toBool(), false);

    // Check if switching the error correction off disables the "select with space"
    subject->setErrorCorrectionState(true);
    subject->setCorrectionSpaceState(true);
    spyErrorCorrection.clear();
    spyCorrectionSpace.clear();
    // Switch off the error correction
    subject->setErrorCorrectionState(false);
    QCOMPARE(spyErrorCorrection.count(), 1);
    QCOMPARE(spyErrorCorrection.takeFirst().at(0).toBool(), false);
    QCOMPARE(errorCorrectionSetting.value().toBool(), false);
    // Check if the "select with space" setting has been disabled
    QCOMPARE(spyCorrectionSpace.count(), 1);
    QCOMPARE(spyCorrectionSpace.takeFirst().at(0).toBool(), false);
    QCOMPARE(correctionSpaceSetting.value().toBool(), false);
}

void Ut_MKeyboardSettingsWidget::testHandleVisibilityChanged()
{
    subject->setVisible(true);
    subject->showKeyboardList();
    QVERIFY(subject->keyboardDialog);
    QCOMPARE(subject->keyboardDialog->isVisible(), true);

    QSignalSpy spy(subject->keyboardDialog, SIGNAL(rejected()));
    subject->setVisible(false);
    QCOMPARE(spy.count(), 1);
}

QTEST_APPLESS_MAIN(Ut_MKeyboardSettingsWidget);
