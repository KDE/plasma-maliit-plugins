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



#include "mimtoolbar.h"
#include "toolbardata.h"
#include "toolbarmanager.h"
#include "layoutsmanager.h"
#include "mhardwarekeyboard.h"
#include "mvirtualkeyboardstyle.h"

#include <MNamespace>
#include <MButton>
#include <QKeySequence>
#include <QGraphicsLinearLayout>
#include <QDebug>
#include <QTimer>
#include <MSceneManager>
#include <mplainwindow.h>
#include <MInfoBanner>
#include <duireactionmap.h>
#include <MScalableImage>

namespace
{
    //!object name for toolbar buttons
    const QString ObjectNameToolbarButtons("VirtualKeyboardToolbarButton");
    const QString ObjectNameCloseButton("VirtualKeyboardCloseButton");
    const QString ObjectNameIndicatorButton("VirtualKeyboardIndicatorButton");
    const QString ObjectNameToolbar("MImToolbar");
    const QString ObjectNameToolbarLeft("VirtualKeyboardToolbarLeft");
    const QString ObjectNameToolbarRight("VirtualKeyboardToolbarRight");
    const QString IconNameCloseButton("icon-m-input-methods-close");
    //! indicator label for latin
    const QString LatinShiftOffIndicatorLabel("abc");
    const QString LatinShiftOnIndicatorLabel("Abc");
    const QString LatinShiftLockedIndicatorLabel("ABC");
    //! indicator label for cyrillic
    const QString CyrillicShiftOffIndicatorLabel = QString("%1%2%3")
            .arg(QChar(0x0430))
            .arg(QChar(0x0431))
            .arg(QChar(0x0432));
    const QString CyrillicShiftOnIndicatorLabel = QString("%1%2%3")
            .arg(QChar(0x0410))
            .arg(QChar(0x0431))
            .arg(QChar(0x0432));
    const QString CyrillicShiftLockedIndicatorLabel = QString("%1%2%3")
            .arg(QChar(0x0410))
            .arg(QChar(0x0411))
            .arg(QChar(0x0412));
    //! indicator label for arabic
    const QString ArabicIndicatorLabel = QString("%1%2%3%4%5")
                                         .arg(QChar(0x0627))
                                         .arg(QChar(0x200C))
                                         .arg(QChar(0x0628))
                                         .arg(QChar(0x200C))
                                         .arg(QChar(0x062A));
    //! indicator label for FN
    const QString FNOnIndicatorLabel("123");
    const QString FNLockedIndicatorLabel("<U>123</U>");
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;
    const int ModifierLockOnInfoDuration = 1000;
};

MImToolbar::MImToolbar(MVirtualKeyboardStyleContainer &style, QGraphicsWidget *parent)
    : MWidget(parent),
      toolbarMgr(new ToolbarManager(this)),
      textSelected(false),
      indicator(new MButton),
      copyPaste(new MButton),
      copyPasteStatus(InputMethodNoCopyPaste),
      leftBar(true, this),
      rightBar(true, this),
      modifierLockOnInfoBanner(0),
      modifierLockOnTimer(new QTimer(this)),
      style(style),
      shiftState(ModifierClearState),
      fnState(ModifierClearState)
{
    // Empty button bars are hidden.
    leftBar.hide();
    rightBar.hide();

    setObjectName(ObjectNameToolbar);
    leftBar.setObjectName(ObjectNameToolbarLeft);
    rightBar.setObjectName(ObjectNameToolbarRight);

    setupLayout();

    loadDefaultButtons();

    connect(this, SIGNAL(visibleChanged()), this, SLOT(updateVisibility()));

    modifierLockOnTimer->setInterval(ModifierLockOnInfoDuration);
    connect(modifierLockOnTimer, SIGNAL(timeout()), this, SLOT(hideLockOnInfoBanner()));
}

MImToolbar::~MImToolbar()
{
    delete toolbarMgr;
    toolbarMgr = 0;
    delete indicator;
    indicator = 0;
    delete copyPaste;
    copyPaste = 0;
    hideLockOnInfoBanner();
}

void MImToolbar::setupLayout()
{
    QGraphicsLinearLayout *mainLayout = new QGraphicsLinearLayout(Qt::Horizontal, this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Add the left and right side ButtonBar widgets with a stretch item in between.
    mainLayout->addItem(&leftBar);
    mainLayout->addStretch();
    mainLayout->addItem(&rightBar);

    mainLayout->setAlignment(&leftBar, Qt::AlignBottom);
    mainLayout->setAlignment(&rightBar, Qt::AlignBottom);

    resize(geometry().width(), layout()->preferredHeight());
}

void MImToolbar::loadDefaultButtons()
{
    // Setup indicator button.
    indicator->setObjectName(ObjectNameToolbarButtons);
    connect(indicator, SIGNAL(clicked()), this, SIGNAL(indicatorClicked()));
    indicator->setVisible(false);

    // Setup copy/paste button.
    copyPaste->setObjectName(ObjectNameToolbarButtons);
    copyPaste->setVisible(false);
    //% "Copy"
    copyPaste->setText(qtTrId("qtn_comm_copy"));
    connect(copyPaste, SIGNAL(clicked()), this, SLOT(copyPasteButtonHandler()));
}

QRegion MImToolbar::region() const
{
    QRegion region;

    if (isVisible()) {
        region = QRegion(mapRectToScene(rect()).toRect());
    }
    return region;
}

void MImToolbar::handleButtonClick()
{
    const MButton *button = qobject_cast<MButton *>(this->sender());
    Q_ASSERT(button);

    const ToolbarButton *toolbarButton = toolbarMgr->toolbarButton(button);
    if (!toolbarButton)
        return;

    foreach(const ToolbarButton::Action * action, toolbarButton->actions) {
        switch (action->type) {
        case ToolbarButton::SendKeySequence:
            sendKeySequence(action->keys);
            break;
        case ToolbarButton::SendString:
            sendStringRequest(action->text);
            break;
        case ToolbarButton::SendCommand:
            //TODO:not support yet
            break;
        case ToolbarButton::Copy:
            emit copyPasteRequest(InputMethodCopy);
            break;
        case ToolbarButton::Paste:
            emit copyPasteRequest(InputMethodPaste);
            break;
        case ToolbarButton::ShowGroup:
            showGroup(action->group);
            break;
        case ToolbarButton::HideGroup:
            hideGroup(action->group);
            break;
        case ToolbarButton::Unknown:
            break;
        }
    }
}

void MImToolbar::setSelectionStatus(bool selection)
{
    if (textSelected != selection) {
        textSelected = selection;
        if (isVisible())
            updateVisibility();
    }
}

void MImToolbar::updateVisibility()
{
    qDebug() << __PRETTY_FUNCTION__;
    //set button visibility according showOn and hideOn premiss and current selection status
    foreach(ToolbarButton * b, toolbarMgr->buttonList()) {
        if ((textSelected && b->hideOn != ToolbarButton::WhenSelectingText)
                || (b->showOn == ToolbarButton::Always)) {
            b->setVisible(true);
        } else {
            b->setVisible(false);
        }
    }

    // Update button widgets according to toolbar models.
    updateButtons();
}

void MImToolbar::loadCustomButtons(Qt::Alignment align)
{
    qDebug() << __PRETTY_FUNCTION__ << align;
    //the buttons gotten from toolbarMgr are already ordered acording their priority with alignment.
    QList<ToolbarButton *> buttons = toolbarMgr->buttonList(align);
    //show buttons according their status and priority
    int buttonCount = 0;
    foreach(ToolbarButton * toolbarButton, buttons) {
        MButton *button = toolbarMgr->button(toolbarButton->name);
        if (!button)
            continue;
        if (toolbarButton->isVisible()) {
            button->setVisible(true);
            //if button is Visible, then insert it to the right position
            insertItem(buttonCount, button, align);
            ++buttonCount;
        } else {
            button->setVisible(false);
            removeItem(button);
        }
    }
}

void MImToolbar::unloadCustomButtons(Qt::Alignment align)
{
    QList<ToolbarButton *> buttons = toolbarMgr->buttonList(align);
    foreach(ToolbarButton * toolbarButton, buttons) {
        MButton *button = toolbarMgr->button(toolbarButton->name);
        if (!button)
            continue;
        button->setVisible(false);
        removeItem(button);
    }
}

void MImToolbar::updateButtons(bool customButtonsChanged)
{
    if (customButtonsChanged) {
        loadCustomButtons(Qt::AlignLeft);
        loadCustomButtons(Qt::AlignRight);
    }

    if (isVisible()) {
        layout()->invalidate();
    }

    emit regionUpdated();
}

void MImToolbar::showGroup(const QString &group)
{
    bool changed = false;
    foreach(ToolbarButton * b, toolbarMgr->buttonList()) {
        if (b->group == group && !(b->isVisible())) {
            b->setVisible(true);
            changed = true;
        }
    }

    if (changed) {
        updateButtons();
    }
}

void MImToolbar::hideGroup(const QString &group)
{
    bool changed = false;
    foreach(ToolbarButton * b, toolbarMgr->buttonList()) {
        if (b->group == group && b->isVisible()) {
            b->setVisible(false);
            changed = true;
        }
    }
    if (changed) {
        updateButtons(true);
    }
}

void MImToolbar::sendKeySequence(const QString &keys)
{
    const QKeySequence keysequence(keys);
    //translate the keys string to QKeyEvent by QKeySequence
    if (!keysequence.isEmpty()) {
        for (uint i = 0; i < keysequence.count(); i++) {
            int key = keysequence[i];
            Qt::KeyboardModifiers modify = keyModifiers(key);
            key -= modify;
            QString text;
            if (modify == Qt::NoModifier || modify == Qt::ShiftModifier)
                text = QString(key);
            //send both KeyPress and KeyRelease
            QKeyEvent press(QEvent::KeyPress, key, modify, text);
            emit sendKeyEventRequest(press);
            QKeyEvent release(QEvent::KeyRelease, key, modify, text);
            emit sendKeyEventRequest(release);
        }
    }
}

Qt::KeyboardModifiers MImToolbar::keyModifiers(int key) const
{
    Qt::KeyboardModifiers modify = Qt::NoModifier;
    if (key & Qt::CTRL)
        modify |= Qt::ControlModifier;
    if (key & Qt::ALT)
        modify |= Qt::AltModifier;
    if (key & Qt::SHIFT)
        modify |= Qt::ShiftModifier;
    if (key & Qt::META)
        modify |= Qt::MetaModifier;
    return modify;
}

void MImToolbar::showToolbarWidget(const QString &name)
{
    qDebug() << __PRETTY_FUNCTION__ << name;
    if (name != toolbarMgr->currentToolbar()) {
        unloadCustomButtons(Qt::AlignLeft);
        unloadCustomButtons(Qt::AlignRight);
    }
    if (toolbarMgr->loadToolbar(name) && isVisible())
        updateVisibility();
}

void MImToolbar::hideToolbarWidget()
{
    qDebug() << __PRETTY_FUNCTION__;
    unloadCustomButtons(Qt::AlignLeft);
    unloadCustomButtons(Qt::AlignRight);
    toolbarMgr->reset();
    if (isVisible())
        updateVisibility();
}

void MImToolbar::hideIndicatorButton()
{
    indicator->setVisible(false);
    removeItem(indicator);
}

void MImToolbar::showIndicatorButton()
{
    if (indicator->isVisible())
        return;

    // Sets default label.
    QString indicatorLabel;
    const QString currentDisplayLanguage = LayoutsManager::instance().systemDisplayLanguage();
    if (currentDisplayLanguage == "ar") {
        indicatorLabel = ArabicIndicatorLabel;
    } else if (LayoutsManager::isCyrillicLanguage(currentDisplayLanguage)) {
        indicatorLabel = CyrillicShiftOffIndicatorLabel;
    } else {
        indicatorLabel = LatinShiftOffIndicatorLabel;
    }
    indicator->setText(indicatorLabel);
    indicator->setVisible(true);
    // indicator is the right most button
    insertItem(rightBar.count(), indicator, Qt::AlignRight);
}

void MImToolbar::copyPasteButtonHandler()
{
    if (copyPasteStatus == InputMethodNoCopyPaste)
        return;

    emit copyPasteClicked(copyPasteStatus);
}

void MImToolbar::setCopyPasteButton(bool copyAvailable, bool pasteAvailable)
{
    CopyPasteState newStatus = InputMethodNoCopyPaste;

    if (copyAvailable) {
        newStatus = InputMethodCopy;
    } else if (pasteAvailable) {
        newStatus = InputMethodPaste;
    }

    if (copyPasteStatus == newStatus)
        return;

    // Show/hide CopyPaste button.
    //copy button is in the front of the indicator if it is shown,
    //otherwise copy button becomes most right button in toolbar
    const int offset = (rightBar.contains(indicator)) ? 1 : 0;
    const int buttonIndex = qMax(0, (rightBar.count() - offset));
    const bool hasCopyPasteButton = rightBar.contains(copyPaste);
    bool changed = false;
    copyPasteStatus = newStatus;
    switch (newStatus) {
    case InputMethodNoCopyPaste:
        if (hasCopyPasteButton) {
            changed = true;
            removeItem(copyPaste);
            copyPaste->setVisible(false);
        }
        break;
    case InputMethodCopy:
        if (!hasCopyPasteButton) {
            changed = true;
            insertItem(buttonIndex, copyPaste, Qt::AlignRight);
        }
        copyPaste->setVisible(true);
        //% "Copy"
        copyPaste->setText(qtTrId("qtn_comm_copy"));
        break;
    case InputMethodPaste:
        if (!hasCopyPasteButton) {
            changed = true;
            insertItem(buttonIndex, copyPaste, Qt::AlignRight);
        }
        copyPaste->setVisible(true);
        //% "Paste"
        copyPaste->setText(qtTrId("qtn_comm_paste"));
        break;
    }
    if (changed) {
        // Update only positions.
        updateButtons(false);
    }
    qDebug() << __PRETTY_FUNCTION__ << copyPaste->isVisible();
}

void MImToolbar::insertItem(const int index, MButton *button, Qt::Alignment align)
{
    Q_ASSERT((align == Qt::AlignLeft) || (align == Qt::AlignRight));
    ButtonBar *sidebar = (align == Qt::AlignLeft) ? &leftBar : &rightBar;
    if (!sidebar->contains(button)) {
        sidebar->insert(index, button);
    }

    if (leftBar.count() == 1) {
        leftBar.show();
    }
    if (rightBar.count() == 1) {
        rightBar.show();
    }

    layout()->invalidate();
    layout()->activate();
}

void MImToolbar::removeItem(MButton *button)
{
    leftBar.remove(button);
    rightBar.remove(button);

    if (leftBar.count() == 0) {
        leftBar.hide();
    }
    if (rightBar.count() == 0) {
        rightBar.hide();
    }

    layout()->invalidate();
    layout()->activate();
}

void MImToolbar::drawReactiveAreas(DuiReactionMap *reactionMap, QGraphicsView *view)
{
    // TODO: support for translucent keyboard
    reactionMap->setTransform(this, view);
    reactionMap->setInactiveDrawingValue();
    reactionMap->fillRectangle(rect());

    // Draw all button geometries.
    reactionMap->setReactiveDrawingValue();

    for (int j = 0; j < 2; ++j) {
        ButtonBar *sidebar = ((j == 0) ? &leftBar : &rightBar);
        if (!sidebar->isVisible()) {
            continue;
        }
        reactionMap->setTransform(sidebar, view);

        for (int i = 0; i < sidebar->count(); ++i) {
            reactionMap->fillRectangle(sidebar->buttonAt(i)->geometry());
        }
    }
}

void MImToolbar::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const MScalableImage *background = style->toolbarBackgroundImage();

    if (background) {
        background->draw(rect().toRect(), painter);
    }
}

void MImToolbar::clearReactiveAreas()
{
    if (!scene())
        return;

    Q_ASSERT(scene()->views().count() == 1);

    QGraphicsView *view = scene()->views()[0];
    DuiReactionMap *reactionMap = DuiReactionMap::instance(view);
    if (!reactionMap)
        return;

    reactionMap->setTransform(this, view);
    reactionMap->setInactiveDrawingValue();
    reactionMap->fillRectangle(leftBar.geometry());
    reactionMap->fillRectangle(rightBar.geometry());
}

void MImToolbar::setIndicatorButtonState(Qt::KeyboardModifier modifier, ModifierState state)
{
    //TODO: consider the indicator label for Arabic and Cyrillic (maybe Chinese also)?
    QString indicatorLabel;
    QString lockOnNotificationLabel;
    const QString currentDisplayLanguage = LayoutsManager::instance().systemDisplayLanguage();
    switch (modifier) {
    case Qt::ShiftModifier:
        shiftState = state;
        switch (state) {
        case ModifierClearState:
            if (fnState != ModifierClearState) {
                return;
            }
            if (currentDisplayLanguage == "ar") {
                indicatorLabel = ArabicIndicatorLabel;
            } else if (LayoutsManager::isCyrillicLanguage(currentDisplayLanguage)) {
                indicatorLabel = CyrillicShiftOffIndicatorLabel;
            } else {
                indicatorLabel = LatinShiftOffIndicatorLabel;
            }
            break;
        case ModifierLatchedState:
            if (currentDisplayLanguage == "ar") {
                indicatorLabel = ArabicIndicatorLabel;
            } else if (LayoutsManager::isCyrillicLanguage(currentDisplayLanguage)) {
                indicatorLabel = CyrillicShiftOnIndicatorLabel;
            } else {
                indicatorLabel = LatinShiftOnIndicatorLabel;
            }
            break;
        case ModifierLockedState:
            if (currentDisplayLanguage == "ar") {
                indicatorLabel = ArabicIndicatorLabel;
            } else if (LayoutsManager::isCyrillicLanguage(currentDisplayLanguage)) {
                indicatorLabel = CyrillicShiftLockedIndicatorLabel;
            } else {
                indicatorLabel = LatinShiftLockedIndicatorLabel;
            }
            //% "Caps lock on"
            lockOnNotificationLabel = qtTrId("qtn_hwkb_caps_lock");
            break;
        }
        break;
    case FnLevelModifier:
        fnState = state;
        switch (state) {
        case ModifierClearState:
            if (shiftState != ModifierClearState) {
                return;
            }
            // when fn key change back to clear, shows default label "abc"
            indicatorLabel = LatinShiftOffIndicatorLabel;
            break;
        case ModifierLatchedState:
            indicatorLabel = FNOnIndicatorLabel;
            break;
        case ModifierLockedState:
            indicatorLabel = FNLockedIndicatorLabel;
            //% "Symbol lock on"
            lockOnNotificationLabel = qtTrId("qtn_hwkb_fn_lock");
            break;
        }
        break;
    default:
        //do not deal with the other modifiers
        return;
    }
    indicator->setText(indicatorLabel);
    if (state == ModifierLockedState) {
        // notify the modifier is changed to locked state
        if (modifierLockOnInfoBanner) {
            modifierLockOnInfoBanner->setBodyText(lockOnNotificationLabel);
            modifierLockOnTimer->start();
        } else {
            modifierLockOnInfoBanner = new MInfoBanner(MInfoBanner::Information);
            modifierLockOnInfoBanner->setBodyText(lockOnNotificationLabel);
            MPlainWindow::instance()->sceneManager()->appearSceneWindow(modifierLockOnInfoBanner,
                    MSceneWindow::DestroyWhenDone);
            modifierLockOnTimer->start();
        }
    } else if (modifierLockOnInfoBanner) {
        hideLockOnInfoBanner();
    }
}

void MImToolbar::hideLockOnInfoBanner()
{
    if (modifierLockOnInfoBanner)
        MPlainWindow::instance()->sceneManager()->disappearSceneWindow(modifierLockOnInfoBanner);
    modifierLockOnInfoBanner = 0;
}
