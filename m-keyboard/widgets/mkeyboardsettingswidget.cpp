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

    QString langId = index.data(Qt::UserRole + 1).toString();
    contentItem->setTitle(langId);
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
    selectedKeyboardsItem = new MContentItem(MContentItem::TwoTextLabels, this);
    selectedKeyboardsItem->setObjectName(ObjectNameSelectedKeyboardsItem);
    connect(selectedKeyboardsItem, SIGNAL(clicked()), this, SLOT(showKeyboardList()));
    addItem(selectedKeyboardsItem);

    errorCorrectionSwitch = new MButton(this);
    errorCorrectionSwitch->setObjectName(ObjectNameErrorCorrectionButton);
    errorCorrectionSwitch->setViewType(MButton::switchType);
    errorCorrectionSwitch->setCheckable(true);
    errorCorrectionLabel = new MLabel(this);
    //% "Error correction"
    errorCorrectionLabel->setText(qtTrId("qtn_txts_error_correction"));
    errorCorrectionLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QGraphicsLinearLayout *l = new QGraphicsLinearLayout(Qt::Horizontal);
    l->addItem(errorCorrectionLabel);
    l->addItem(errorCorrectionSwitch);
    l->setAlignment(errorCorrectionSwitch, Qt::AlignCenter);
    (qobject_cast<MKeyboardSettingsWidget *>(this))->addItem(l);
}

void MKeyboardSettingsWidget::addItem(QGraphicsLayoutItem *item)
{
    int count = landscapePolicy->count();
    int row = count / 2;
    int column = count % 2;

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
    if (!errorCorrectionLabel || !settingsObject || !selectedKeyboardsItem)
        return;
    //% "Error correction"
    errorCorrectionLabel->setText(qtTrId("qtn_txts_error_correction"));
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

    if (!settingsObject || !errorCorrectionSwitch)
        return;

    connect(errorCorrectionSwitch, SIGNAL(toggled(bool)),
            this, SLOT(setErrorCorrectionState(bool)));
    connect(settingsObject, SIGNAL(errorCorrectionChanged()),
            this, SLOT(syncErrorCorrectionState()));
    connect(settingsObject, SIGNAL(selectedKeyboardsChanged()),
            this, SLOT(updateTitle()));
    connect(settingsObject, SIGNAL(selectedKeyboardsChanged()),
            this, SLOT(updateKeyboardSelectionModel()));
}

void MKeyboardSettingsWidget::showKeyboardList()
{
    if (!settingsObject || !keyboardDialog) {
        QStringList keyboards = settingsObject->selectedKeyboards().values();
        //% "Installed keyboards (%1)"
        QString keyboardTitle = qtTrId("qtn_txts_installed_keyboards")
                                       .arg(keyboards.count());
        keyboardDialog = new MDialog(keyboardTitle, M::NoStandardButton);

        keyboardList = new MList(keyboardDialog);
        MKeyboardCellCreator *cellCreator = new MKeyboardCellCreator();
        keyboardList->setCellCreator(cellCreator);
        QStandardItemModel *model = new QStandardItemModel();
        model->sort(0);
        keyboardList->setItemModel(model);
        keyboardList->setSelectionMode(MList::MultiSelection);
        keyboardList->setSelectionModel(new QItemSelectionModel(model, this));
        keyboardDialog->setCentralWidget(keyboardList);

        connect(keyboardList, SIGNAL(itemClicked(const QModelIndex &)),
                this, SLOT(updateSelectedKeyboards(const QModelIndex &)));
    }
    updateKeyboardModel();
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
    foreach (QString keyboard, settingsObject->availableKeyboards()) {
        QStandardItem *item = new QStandardItem(keyboard);
        item->setData(keyboard);
        model->appendRow(item);
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
    if (!settingsObject || !index.isValid() || !keyboardList
        || !keyboardList->selectionModel())
        return;

    QStringList updatedKeyboardTitles;
    foreach (const QModelIndex &i, keyboardList->selectionModel()->selectedIndexes()) {
        updatedKeyboardTitles << i.data(Qt::DisplayRole).toString();
    }
    if (updatedKeyboardTitles.isEmpty()) {
        notifyNoKeyboards();
    }
    settingsObject->setSelectedKeyboards(updatedKeyboardTitles);
    //update titles
    retranslateUi();
}

void MKeyboardSettingsWidget::setErrorCorrectionState(bool toggled)
{
    if (!settingsObject)
        return;
    if (toggled != settingsObject->errorCorrection())
        settingsObject->setErrorCorrection(toggled) ;
}

void MKeyboardSettingsWidget::syncErrorCorrectionState()
{
    if (!settingsObject)
        return;
    const bool errorCorrectionState = settingsObject->errorCorrection();
    if (errorCorrectionSwitch
        && errorCorrectionSwitch->isChecked() != errorCorrectionState) {
        errorCorrectionSwitch->setChecked(errorCorrectionState);
    }
}

void MKeyboardSettingsWidget::notifyNoKeyboards()
{
    MBanner *noKeyboardsNotification = new MBanner();
    //% "No keyboards installed"
    noKeyboardsNotification->setTitle(qtTrId("qtn_txts_no_keyboards"));
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
