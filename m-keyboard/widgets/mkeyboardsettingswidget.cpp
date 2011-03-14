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

#include "mkeyboardsettingswidget.h"

#include <MButton>
#include <MLabel>
#include <MLayout>
#include <MLocale>
#include <MGridLayoutPolicy>
#include <MLinearLayoutPolicy>
#include <MContentItem>
#include <MAbstractCellCreator>
#include <MList>
#include <MDialog>
#include <MBanner>
#include <MBasicListItem>

#include <QObject>
#include <QGraphicsLinearLayout>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QTimer>
#include <QDebug>

namespace
{
    //!object name for settings' widgets
    const QString ObjectNameSelectedKeyboardsItem("SelectedKeyboardsItem");
    const QString ObjectNameErrorCorrectionButton("KeyboardErrorCorrectionButton");
    const QString ObjectNameCorrectionSpaceButton("KeyboardCorrectionSpaceButton");
    const int MKeyboardLayoutRole = Qt::UserRole + 1;
};

class MKeyboardCellCreator: public MAbstractCellCreator<MContentItem>
{
public:
    /*! \reimp */
    virtual MWidget *createCell (const QModelIndex &index,
                                 MWidgetRecycler &recycler) const;
    virtual void updateCell(const QModelIndex &index, MWidget *cell) const;
    /*! \reimp_end */
private:
    void updateContentItemMode(const QModelIndex &index, MContentItem *contentItem) const;
};

MWidget *MKeyboardCellCreator::createCell (const QModelIndex &index,
                                           MWidgetRecycler &recycler) const
{
    MContentItem *cell = qobject_cast<MContentItem *>(recycler.take("MContentItem"));
    if (!cell) {
        cell = new MContentItem(MContentItem::SingleTextLabel);
    }
    updateCell(index, cell);
    return cell;
}

void MKeyboardCellCreator::updateCell(const QModelIndex &index, MWidget *cell) const
{
    MContentItem *contentItem = qobject_cast<MContentItem *>(cell);

    QString layoutTile = index.data(Qt::DisplayRole).toString();
    contentItem->setTitle(layoutTile);
}

void MKeyboardCellCreator::updateContentItemMode(const QModelIndex &index,
                                                 MContentItem *contentItem) const
{
    const int row = index.row();
    bool thereIsNextRow = index.sibling(row + 1, 0).isValid();
    if (row == 0) {
        contentItem->setItemMode(MContentItem::SingleColumnTop);
    } else if (thereIsNextRow) {
        contentItem->setItemMode(MContentItem::SingleColumnCenter);
    } else {
        contentItem->setItemMode(MContentItem::SingleColumnBottom);
    }
}

MKeyboardSettingsWidget::MKeyboardSettingsWidget(MKeyboardSettings *settings, QGraphicsItem *parent)
    : MWidget(parent),
      settingsObject(settings),
      keyboardDialog(0),
      keyboardList(0)
{
    MLayout *layout = new MLayout(this);

    landscapePolicy = new MGridLayoutPolicy(layout);
    landscapePolicy->setContentsMargins(0, 0, 0, 0);
    landscapePolicy->setSpacing(0);
    //To make sure that both columns have the same width, give them the same preferred width.
    landscapePolicy->setColumnPreferredWidth(0, 800);
    landscapePolicy->setColumnPreferredWidth(1, 800);
    portraitPolicy = new MLinearLayoutPolicy(layout, Qt::Vertical);
    portraitPolicy->setContentsMargins(0, 0, 0, 0);
    portraitPolicy->setSpacing(0);

    layout->setLandscapePolicy(landscapePolicy);
    layout->setPortraitPolicy(portraitPolicy);

    buildUi();
    syncErrorCorrectionState();
    syncCorrectionSpaceState();
    retranslateUi();
    connectSlots();
}

MKeyboardSettingsWidget::~MKeyboardSettingsWidget()
{
    delete keyboardDialog;
    keyboardDialog = 0;
}

void MKeyboardSettingsWidget::buildUi()
{
    // We are using MBasicListItem instead of MContentItem because
    // the latter is not supported by theme
    selectedKeyboardsItem = new MBasicListItem(MBasicListItem::TitleWithSubtitle, this);
    selectedKeyboardsItem->setObjectName(ObjectNameSelectedKeyboardsItem);
    connect(selectedKeyboardsItem, SIGNAL(clicked()), this, SLOT(showKeyboardList()));
    selectedKeyboardsItem->setStyleName("CommonBasicListItemInverted");

    // Put to first row, first column on the grid
    addItem(selectedKeyboardsItem, 0, 0);

    // Error correction settings
    errorCorrectionSwitch = new MButton(this);
    errorCorrectionSwitch->setObjectName(ObjectNameErrorCorrectionButton);
    errorCorrectionSwitch->setViewType(MButton::switchType);
    errorCorrectionSwitch->setCheckable(true);
    errorCorrectionContentItem = new MBasicListItem(MBasicListItem::TitleWithSubtitle, this);
    errorCorrectionContentItem->setStyleName("CommonBasicListItemInverted");
    //% "Error correction"
    errorCorrectionContentItem->setTitle(qtTrId("qtn_txts_error_correction"));
    //% "Error correction description"
    errorCorrectionContentItem->setSubtitle(qtTrId("qtn_txts_error_correction_description"));
    QGraphicsLinearLayout *eCLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    eCLayout->addItem(errorCorrectionContentItem);
    eCLayout->addItem(errorCorrectionSwitch);
    eCLayout->setAlignment(errorCorrectionSwitch, Qt::AlignCenter);
    // Put to first row, second column on the grid
    addItem(eCLayout, 0, 1);

    // "Space selects the correction candidate" settings
    correctionSpaceSwitch = new MButton(this);
    correctionSpaceSwitch->setObjectName(ObjectNameCorrectionSpaceButton);
    correctionSpaceSwitch->setViewType(MButton::switchType);
    correctionSpaceSwitch->setCheckable(true);
    correctionSpaceContentItem = new MBasicListItem(MBasicListItem::TitleWithSubtitle, this);
    correctionSpaceContentItem->setStyleName("CommonBasicListItemInverted");
    //% "Insert with space"
    correctionSpaceContentItem->setTitle(qtTrId("qtn_txts_insert_with_space"));
    //% "Space key inserts the suggested word"
    correctionSpaceContentItem->setSubtitle(qtTrId("qtn_txts_insert_with_space_description"));
    QGraphicsLinearLayout *wCLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    wCLayout->addItem(correctionSpaceContentItem);
    wCLayout->addItem(correctionSpaceSwitch);
    wCLayout->setAlignment(correctionSpaceSwitch, Qt::AlignCenter);
    // Put to second row, second column on the grid
    addItem(wCLayout, 1, 1);
}

void MKeyboardSettingsWidget::addItem(QGraphicsLayoutItem *item, int row, int column)
{
    landscapePolicy->addItem(item, row, column);
    portraitPolicy->addItem(item);
}

void MKeyboardSettingsWidget::retranslateUi()
{
    updateTitle();
    MWidget::retranslateUi();
}

void MKeyboardSettingsWidget::updateTitle()
{
    if (!errorCorrectionContentItem || !correctionSpaceContentItem
        || !settingsObject || !selectedKeyboardsItem)
        return;

    errorCorrectionContentItem->setTitle(qtTrId("qtn_txts_error_correction"));
    errorCorrectionContentItem->setSubtitle(qtTrId("qtn_txts_error_correction_description"));
    correctionSpaceContentItem->setTitle(qtTrId("qtn_txts_insert_with_space"));
    correctionSpaceContentItem->setSubtitle(qtTrId("qtn_txts_insert_with_space_description"));
    QStringList keyboards = settingsObject->selectedKeyboards().values();
    //% "Installed keyboards (%1)"
    QString title = qtTrId("qtn_txts_installed_keyboards")
                            .arg(keyboards.count());
    selectedKeyboardsItem->setTitle(title);
    QString brief;
    if (keyboards.count() > 0) {
        foreach(const QString &keyboard, keyboards) {
            if (!brief.isEmpty())
                brief += QString(", ");
            brief += keyboard;
        }
    } else {
        //% "No keyboards installed"
        brief = qtTrId("qtn_txts_no_keyboards");
    }
    selectedKeyboardsItem->setSubtitle(brief);

    if (keyboardDialog) {
        keyboardDialog->setTitle(title);
    }
}

void MKeyboardSettingsWidget::connectSlots()
{
    connect(this, SIGNAL(visibleChanged()),
            this, SLOT(handleVisibilityChanged()));

    if (!settingsObject || !errorCorrectionSwitch || !correctionSpaceSwitch)
        return;

    connect(errorCorrectionSwitch, SIGNAL(toggled(bool)),
            this, SLOT(setErrorCorrectionState(bool)));
    connect(settingsObject, SIGNAL(errorCorrectionChanged()),
            this, SLOT(syncErrorCorrectionState()));
    connect(correctionSpaceSwitch, SIGNAL(toggled(bool)),
            this, SLOT(setCorrectionSpaceState(bool)));
    connect(settingsObject, SIGNAL(correctionSpaceChanged()),
            this, SLOT(syncCorrectionSpaceState()));
    connect(settingsObject, SIGNAL(selectedKeyboardsChanged()),
            this, SLOT(updateKeyboardSelectionModel()));
}

void MKeyboardSettingsWidget::showKeyboardList()
{
    if (!settingsObject)
        return;

    QStringList keyboards = settingsObject->selectedKeyboards().values();

    if (!keyboardDialog) {
        keyboardDialog = new MDialog();

        keyboardList = new MList(keyboardDialog);
        MKeyboardCellCreator *cellCreator = new MKeyboardCellCreator;
        keyboardList->setCellCreator(cellCreator);
        QStandardItemModel *model = new QStandardItemModel;
        model->sort(0);
        keyboardList->setItemModel(model);
        keyboardList->setSelectionMode(MList::MultiSelection);
        keyboardList->setSelectionModel(new QItemSelectionModel(model, this));
        keyboardDialog->setCentralWidget(keyboardList);
        keyboardDialog->addButton(M::DoneButton);

        connect(keyboardList, SIGNAL(itemClicked(const QModelIndex &)),
                this, SLOT(updateSelectedKeyboards(const QModelIndex &)));
        connect(keyboardDialog, SIGNAL(accepted()),
                this, SLOT(selectKeyboards()));
    }
    updateKeyboardModel();
    // We need to update the title every time because probably the dialog was
    // cancelled/closed without tapping on the Done button.
    QString keyboardTitle = qtTrId("qtn_txts_installed_keyboards")
                                   .arg(keyboards.count());
    keyboardDialog->setTitle(keyboardTitle);
    keyboardDialog->exec();
}

void MKeyboardSettingsWidget::updateKeyboardModel()
{
    if (!settingsObject || !keyboardList)
        return;

    //always reload available layouts in case user install/remove some layouts
    settingsObject->readAvailableKeyboards();
    QStandardItemModel *model = static_cast<QStandardItemModel*> (keyboardList->itemModel());
    model->clear();

    QMap<QString, QString> availableKeyboards = settingsObject->availableKeyboards();
    QMap<QString, QString>::const_iterator i = availableKeyboards.constBegin();
    while (i != availableKeyboards.constEnd()) {
        QStandardItem *item = new QStandardItem(i.value());
        item->setData(i.value(), Qt::DisplayRole);
        item->setData(i.key(), MKeyboardLayoutRole);
        model->appendRow(item);
        ++i;
    }
    updateKeyboardSelectionModel();
}

void MKeyboardSettingsWidget::updateKeyboardSelectionModel()
{
    if (!settingsObject || !keyboardList)
        return;

    QStandardItemModel *model = static_cast<QStandardItemModel*> (keyboardList->itemModel());
    foreach (const QString &keyboard, settingsObject->selectedKeyboards().values()) {
        QList<QStandardItem *> items = model->findItems(keyboard);
        foreach (const QStandardItem *item, items) {
            keyboardList->selectionModel()->select(item->index(), QItemSelectionModel::Select);
        }
    }
}

void MKeyboardSettingsWidget::updateSelectedKeyboards(const QModelIndex &index)
{
    if (!index.isValid() || !keyboardDialog || !keyboardList
        || !keyboardList->selectionModel())
        return;

    QModelIndexList indexList = keyboardList->selectionModel()->selectedIndexes();

    // Update the dialog title
    QString title = qtTrId("qtn_txts_installed_keyboards")
                            .arg(indexList.size());

    keyboardDialog->setTitle(title);
}

void MKeyboardSettingsWidget::selectKeyboards()
{
    if (!settingsObject || !keyboardDialog)
        return;

    QStringList updatedKeyboardLayouts;
    QModelIndexList indexList = keyboardList->selectionModel()->selectedIndexes();

    foreach (const QModelIndex &i, indexList) {
        updatedKeyboardLayouts << i.data(MKeyboardLayoutRole).toString();
    }
    settingsObject->setSelectedKeyboards(updatedKeyboardLayouts);
    // "No keyboard is selected" notification
    if (indexList.isEmpty()) {
        notifyNoKeyboards();
    }
    //update titles
    retranslateUi();
}

void MKeyboardSettingsWidget::setErrorCorrectionState(bool enabled)
{
    if (!settingsObject)
        return;

    if (settingsObject->errorCorrection() != enabled) {
        settingsObject->setErrorCorrection(enabled);
        if (!enabled) {
            // Disable the "Select with Space" option if the error correction is disabled
            setCorrectionSpaceState(false);
            correctionSpaceSwitch->setEnabled(false);
        } else {
            // Enable the "Select with Space" switch again
            correctionSpaceSwitch->setEnabled(true);
        }
    }
}

void MKeyboardSettingsWidget::syncErrorCorrectionState()
{
    if (!settingsObject)
        return;

    const bool errorCorrectionState = settingsObject->errorCorrection();
    if (errorCorrectionSwitch
        && errorCorrectionSwitch->isChecked() != errorCorrectionState) {
        errorCorrectionSwitch->setChecked(errorCorrectionState);
        if (!errorCorrectionState) {
            // Disable the "Select with Space" option if the error correction is disabled
            setCorrectionSpaceState(false);
            correctionSpaceSwitch->setEnabled(false);
        } else {
            // Enable the "Select with Space" switch again
            correctionSpaceSwitch->setEnabled(true);
        }
    }
}

void MKeyboardSettingsWidget::setCorrectionSpaceState(bool enabled)
{
    if (!settingsObject)
        return;

    if (settingsObject->correctionSpace() != enabled)
        settingsObject->setCorrectionSpace(enabled);
}

void MKeyboardSettingsWidget::syncCorrectionSpaceState()
{
    if (!settingsObject)
        return;

    const bool correctionSpaceState = settingsObject->correctionSpace();
    if (correctionSpaceSwitch
        && correctionSpaceSwitch->isChecked() != correctionSpaceState) {
        correctionSpaceSwitch->setChecked(correctionSpaceState);
    }
}

void MKeyboardSettingsWidget::notifyNoKeyboards()
{
    MBanner *noKeyboardsNotification = new MBanner;

    // It is needed to set the proper style name to have properly wrapped, multiple lines
    // with too much content. The MBanner documentation also emphasises to specify the
    // style name for the banners explicitly in the code.
    noKeyboardsNotification->setStyleName("InformationBanner");
    //% "Note: you have uninstalled all virtual keyboards"
    noKeyboardsNotification->setTitle(qtTrId("qtn_txts_no_keyboards_notification"));
    noKeyboardsNotification->appear(MSceneWindow::DestroyWhenDone);
}

void MKeyboardSettingsWidget::handleVisibilityChanged()
{
    // This is a workaround to hide settings dialog when keyboard is hidden.
    // And it could be removed when NB#177922 is fixed.
    if (!isVisible() && keyboardDialog) {
        // reject settings dialog if the visibility of settings widget
        // is changed from shown to hidden.
        keyboardDialog->reject();
    }
}
