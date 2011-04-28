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

#include "mkeyboardsettingswidget.h"
#include "layoutsmanager.h"

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
#include <MContainer>

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
    const QString ObjectNameFuzzyPinyinButton("KeyboardFuzzyPinyinButton");
    const QString ObjectNameWordPredictionButton("KeyboardWordPredictionButton");
    const int MKeyboardLayoutRole = Qt::UserRole + 1;

    const QString ChineseInputLanguage("zh_cn_*.xml");
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
    syncFuzzyState();
    syncWordPredictionState();
    retranslateUi();
    updateChineseSettingPanel();
    connectSlots();
}

MKeyboardSettingsWidget::~MKeyboardSettingsWidget()
{
    if (!keyboardDialog.isNull())
        delete keyboardDialog.data();
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
    errorCorrectionSwitch->setStyleName("CommonRightSwitchInverted");
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
    correctionSpaceSwitch->setStyleName("CommonRightSwitchInverted");
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

    // Initial Chinese setting

    // Chinese setting panel container
    QGraphicsLinearLayout *containerLayout = new QGraphicsLinearLayout(Qt::Vertical);
    // Ensure that there is no offset compared to widgets in layout above
    containerLayout->setContentsMargins(0.0, 0.0, 0.0, 0.0);
    chineseContainer = new QGraphicsWidget(this); // Used to be able to show/hide all children
    chineseContainer->setLayout(containerLayout);
    //% "Chinese virtual keyboards"
    chineseSettingHeader = new MLabel(qtTrId("qtn_ckb_chinese_keyboards"), this);
    chineseSettingHeader->setStyleName("CommonGroupHeaderInverted");
    containerLayout->addItem(chineseSettingHeader);

    // Error correction setting
    fuzzySwitch = new MButton(this);
    fuzzySwitch->setObjectName(ObjectNameFuzzyPinyinButton);
    fuzzySwitch->setStyleName("CommonRightSwitchInverted");
    fuzzySwitch->setViewType(MButton::switchType);
    fuzzySwitch->setCheckable(true);
    fuzzyItem = new MBasicListItem(MBasicListItem::SingleTitle, this);
    fuzzyItem->setStyleName("CommonBasicListItemInverted");
    //% "Fuzzy Pinyin input"
    fuzzyItem->setTitle(qtTrId("qtn_ckb_fuzzy_pinyin"));
    QGraphicsLinearLayout *fuzzyLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    fuzzyLayout->addItem(fuzzyItem);
    fuzzyLayout->addItem(fuzzySwitch);
    fuzzyLayout->setAlignment(fuzzySwitch, Qt::AlignCenter);
    containerLayout->addItem(fuzzyLayout);

    // Word prediction setting
    wordPredictionSwitch = new MButton(this);
    wordPredictionSwitch->setObjectName(ObjectNameWordPredictionButton);
    wordPredictionSwitch->setStyleName("CommonRightSwitchInverted");
    wordPredictionSwitch->setViewType(MButton::switchType);
    wordPredictionSwitch->setCheckable(true);
    wordPredictionItem = new MBasicListItem(MBasicListItem::SingleTitle, this);
    wordPredictionItem->setStyleName("CommonBasicListItemInverted");
    //% "Next word prediction"
    wordPredictionItem->setTitle(qtTrId("qtn_ckb_next_prediction"));
    QGraphicsLinearLayout *wordPredictionLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    wordPredictionLayout->addItem(wordPredictionItem);
    wordPredictionLayout->addItem(wordPredictionSwitch);
    wordPredictionLayout->setAlignment(wordPredictionSwitch, Qt::AlignCenter);
    containerLayout->addItem(wordPredictionLayout);

    chineseContainer->setVisible(false);
}

void MKeyboardSettingsWidget::addItem(QGraphicsLayoutItem *item, int row, int column)
{
    landscapePolicy->addItem(item, row, column);
    portraitPolicy->addItem(item);
}

void MKeyboardSettingsWidget::removeItem(QGraphicsLayoutItem *item)
{
    landscapePolicy->removeItem(item);
    portraitPolicy->removeItem(item);
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

    chineseSettingHeader->setText(qtTrId("qtn_ckb_chinese_keyboards"));
    fuzzyItem->setTitle(qtTrId("qtn_ckb_fuzzy_pinyin"));
    wordPredictionItem->setTitle(qtTrId("qtn_ckb_next_prediction"));

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

    if (!keyboardDialog.isNull()) {
        keyboardDialog.data()->setTitle(title);
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

    connect(fuzzySwitch, SIGNAL(toggled(bool)),
            this, SLOT(setFuzzyState(bool)));
    connect(settingsObject, SIGNAL(fuzzyStateChanged()),
            this, SLOT(syncFuzzyState()));

    connect(wordPredictionSwitch, SIGNAL(toggled(bool)),
            this, SLOT(setWordPredictionState(bool)));
    connect(settingsObject, SIGNAL(wordPredictionStateChanged()),
            this, SLOT(syncWordPredictionState()));
}

void MKeyboardSettingsWidget::updateChineseSettingPanel()
{
    QStringList allKeyboardLayoutFiles = settingsObject->selectedKeyboards().keys();

    QRegExp chineseLayoutExp(ChineseInputLanguage, Qt::CaseInsensitive);
    chineseLayoutExp.setPatternSyntax(QRegExp::Wildcard);

    if (allKeyboardLayoutFiles.indexOf(chineseLayoutExp) != -1) {
        // Chinese keyboard installed, then show Chinese setting panel.
        if (!chineseContainer->isVisible()) {
            addItem(chineseContainer, 2, 1);
            chineseContainer->setVisible(true);
        }
    } else {
        // No Chinese keyboard installed, then hide Chinese setting panel.
        if (chineseContainer->isVisible()) {
            removeItem(chineseContainer);
            chineseContainer->setVisible(false);
        }
    }
}

void MKeyboardSettingsWidget::showKeyboardList()
{
    if (!settingsObject)
        return;

    QStringList keyboards = settingsObject->selectedKeyboards().values();

    if (keyboardDialog.isNull()) {
        keyboardDialog = new MDialog();

        keyboardList = new MList(keyboardDialog.data());
        MKeyboardCellCreator *cellCreator = new MKeyboardCellCreator;
        keyboardList->setCellCreator(cellCreator);
        keyboardList->setSelectionMode(MList::MultiSelection);
        createKeyboardModel();
        keyboardDialog.data()->setCentralWidget(keyboardList);
        keyboardDialog.data()->addButton(M::DoneButton);

        connect(keyboardList, SIGNAL(itemClicked(const QModelIndex &)),
                this, SLOT(updateSelectedKeyboards(const QModelIndex &)));
        connect(keyboardDialog.data(), SIGNAL(accepted()),
                this, SLOT(selectKeyboards()));
    }
    updateKeyboardSelectionModel();

    // We need to update the title every time because probably the dialog was
    // cancelled/closed without tapping on the Done button.
    QString keyboardTitle = qtTrId("qtn_txts_installed_keyboards")
                                   .arg(keyboards.count());
    keyboardDialog.data()->setTitle(keyboardTitle);
    keyboardDialog.data()->appear(MSceneWindow::DestroyWhenDone);
}

void MKeyboardSettingsWidget::createKeyboardModel()
{
    if (!settingsObject || !keyboardList)
        return;

    QMap<QString, QString> availableKeyboards = settingsObject->availableKeyboards();

    QStandardItemModel *model = new QStandardItemModel(availableKeyboards.size(), 1, keyboardList);
    QMap<QString, QString>::const_iterator i = availableKeyboards.constBegin();
    for (int j = 0; i != availableKeyboards.constEnd(); ++i, ++j) {
        QStandardItem *item = new QStandardItem(i.value());
        item->setData(i.value(), Qt::DisplayRole);
        item->setData(i.key(), MKeyboardLayoutRole);
        model->setItem(j, item);
    }
    model->sort(0);

    keyboardList->setItemModel(model);
    keyboardList->setSelectionModel(new QItemSelectionModel(model, keyboardList));

    updateKeyboardSelectionModel();
}

void MKeyboardSettingsWidget::updateKeyboardSelectionModel()
{
    if (!settingsObject || !keyboardList)
        return;

    // First clear current selection
    keyboardList->selectionModel()->clearSelection();

    // Select all selected keyboards
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
    if (!index.isValid() || keyboardDialog.isNull() || !keyboardList
        || !keyboardList->selectionModel())
        return;

    QModelIndexList indexList = keyboardList->selectionModel()->selectedIndexes();

    // Update the dialog title
    QString title = qtTrId("qtn_txts_installed_keyboards")
                            .arg(indexList.size());

    keyboardDialog.data()->setTitle(title);
}

void MKeyboardSettingsWidget::selectKeyboards()
{
    if (!settingsObject || keyboardDialog.isNull())
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
    updateChineseSettingPanel();
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
    if (!settingsObject || !errorCorrectionSwitch)
        return;

    const bool errorCorrectionState = settingsObject->errorCorrection();
    if (errorCorrectionSwitch->isChecked() != errorCorrectionState) {
        errorCorrectionSwitch->setChecked(errorCorrectionState);
    }

    if (!errorCorrectionState) {
        // Disable the "Select with Space" option if the error correction is disabled
        setCorrectionSpaceState(false);
        correctionSpaceSwitch->setEnabled(false);
    } else {
        // Enable the "Select with Space" switch again
        correctionSpaceSwitch->setEnabled(true);
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
    if (!isVisible() && !keyboardDialog.isNull()) {
        // reject settings dialog if the visibility of settings widget
        // is changed from shown to hidden.
        keyboardDialog.data()->reject();
    }
}

void MKeyboardSettingsWidget::setFuzzyState(bool enabled)
{
    if (!settingsObject)
          return;
    settingsObject->setFuzzyPinyin(enabled) ;
}

void MKeyboardSettingsWidget::syncFuzzyState()
{
    if (!settingsObject)
        return;

    const bool fuzzyState = settingsObject->fuzzyPinyin();
    if (fuzzySwitch &&
        (fuzzySwitch->isChecked() != fuzzyState)) {
        fuzzySwitch->setChecked(fuzzyState);
    }
}

void MKeyboardSettingsWidget::setWordPredictionState(bool enabled)
{
    if (!settingsObject)
          return;
    settingsObject->setWordPrediction(enabled);
}

void MKeyboardSettingsWidget::syncWordPredictionState()
{
    if (!settingsObject)
        return;

    const bool wordPredictionState = settingsObject->wordPrediction();
    if (wordPredictionSwitch &&
        (wordPredictionSwitch->isChecked() != wordPredictionState)) {
        wordPredictionSwitch->setChecked(wordPredictionState);
    }
}
