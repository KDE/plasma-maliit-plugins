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
    const QString SettingsIMCorrectionSetting("/meegotouch/inputmethods/correctionenabled");
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
                             (char *) "-local-theme" };
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

void Ut_MKeyboardSettingsWidget::testKeyboardErrorCorrection()
{
    MGConfItem errorCorrectionSetting(SettingsIMCorrectionSetting);

    QVERIFY(subject->errorCorrectionSwitch);
    settingsObject->setErrorCorrection(true);
    QCOMPARE(subject->errorCorrectionSwitch->isChecked(), true);
    settingsObject->setErrorCorrection(false);
    QCOMPARE(subject->errorCorrectionSwitch->isChecked(), false);

    QSignalSpy spy(subject->errorCorrectionSwitch, SIGNAL(toggled(bool)));
    subject->setErrorCorrectionState(true);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toBool(), true);
    QCOMPARE(errorCorrectionSetting.value().toBool(), true);

    spy.clear();
    subject->setErrorCorrectionState(false);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toBool(), false);
    QCOMPARE(errorCorrectionSetting.value().toBool(), false);
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
