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
    const QString ObjectNameWordCompletionButton("KeyboardWordCompletionButton");
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

    // Error correction settings
    errorCorrectionSwitch = new MButton(this);
    errorCorrectionSwitch->setObjectName(ObjectNameWordCompletionButton);
    errorCorrectionSwitch->setViewType(MButton::switchType);
    errorCorrectionSwitch->setCheckable(true);
    errorCorrectionLabel = new MLabel(this);
    //% "Error correction"
    errorCorrectionLabel->setText(qtTrId("qtn_txts_error_correction"));
    errorCorrectionLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QGraphicsLinearLayout *eCLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    eCLayout->addItem(errorCorrectionLabel);
    eCLayout->addItem(errorCorrectionSwitch);
    eCLayout->setAlignment(errorCorrectionSwitch, Qt::AlignCenter);

    // Word completion settings
    wordCompletionSwitch = new MButton(this);
    wordCompletionSwitch->setObjectName(ObjectNameErrorCorrectionButton);
    wordCompletionSwitch->setViewType(MButton::switchType);
    wordCompletionSwitch->setCheckable(true);
    wordCompletionLabel = new MLabel(this);
    //% "Word completion"
    wordCompletionLabel->setText(qtTrId("qtn_txts_word_completion"));
    wordCompletionLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QGraphicsLinearLayout *wCLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    wCLayout->addItem(wordCompletionLabel);
    wCLayout->addItem(wordCompletionSwitch);
    wCLayout->setAlignment(wordCompletionSwitch, Qt::AlignCenter);

    // Add the error correction+word completion widgets to this layout to
    // have proper alignment of the widgets to the right side.
    QGraphicsLinearLayout *vertLayout = new QGraphicsLinearLayout(Qt::Vertical);

    // Add the error correction widgets to a vertical layout
    vertLayout->addItem(eCLayout);
    // Add the word completion widgets to a vertical layout
    vertLayout->addItem(wCLayout);
    addItem(vertLayout);
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
    if (!errorCorrectionLabel || !wordCompletionLabel
        || !settingsObject || !selectedKeyboardsItem)
        return;

    //% "Error correction"
    errorCorrectionLabel->setText(qtTrId("qtn_txts_error_correction"));
    //% "Word completion"
    wordCompletionLabel->setText(qtTrId("qtn_txts_word_completion"));
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

    if (!settingsObject || !errorCorrectionSwitch || !wordCompletionSwitch)
        return;

    connect(errorCorrectionSwitch, SIGNAL(toggled(bool)),
            this, SLOT(setErrorCorrectionState(bool)));
    connect(settingsObject, SIGNAL(errorCorrectionChanged()),
            this, SLOT(syncErrorCorrectionState()));
    connect(wordCompletionSwitch, SIGNAL(toggled(bool)),
            this, SLOT(setWordCompletionState(bool)));
    connect(settingsObject, SIGNAL(wordCompletionChanged()),
            this, SLOT(syncWordCompletionState()));
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
        MKeyboardCellCreator *cellCreator = new MKeyboardCellCreator;
        keyboardList->setCellCreator(cellCreator);
        QStandardItemModel *model = new QStandardItemModel;
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
    if (!settingsObject || !index.isValid() || !keyboardList
        || !keyboardList->selectionModel())
        return;

    QStringList updatedKeyboardLayouts;
    foreach (const QModelIndex &i, keyboardList->selectionModel()->selectedIndexes()) {
        updatedKeyboardLayouts << i.data(MKeyboardLayoutRole).toString();
    }
    if (updatedKeyboardLayouts.isEmpty()) {
        notifyNoKeyboards();
    }
    settingsObject->setSelectedKeyboards(updatedKeyboardLayouts);
    //update titles
    retranslateUi();
}

void MKeyboardSettingsWidget::setErrorCorrectionState(bool enabled)
{
    if (!settingsObject)
        return;

    if (settingsObject->errorCorrection() != enabled)
        settingsObject->setErrorCorrection(enabled) ;
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

void MKeyboardSettingsWidget::setWordCompletionState(bool enabled)
{
    if (!settingsObject)
        return;

    if (settingsObject->wordCompletion() != enabled)
        settingsObject->setWordCompletion(enabled) ;
}

void MKeyboardSettingsWidget::syncWordCompletionState()
{
    if (!settingsObject)
        return;

    const bool wordCompletionState = settingsObject->wordCompletion();
    if (wordCompletionSwitch
        && wordCompletionSwitch->isChecked() != wordCompletionState) {
        wordCompletionSwitch->setChecked(wordCompletionState);
    }
}

void MKeyboardSettingsWidget::notifyNoKeyboards()
{
    MBanner *noKeyboardsNotification = new MBanner;
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
