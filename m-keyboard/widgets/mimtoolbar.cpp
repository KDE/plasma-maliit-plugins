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



#include "mimtoolbar.h"
#include "toolbardata.h"
#include "toolbarmanager.h"
#include "toolbarwidget.h"
#include "layoutsmanager.h"
#include "mhardwarekeyboard.h"
#include "mvirtualkeyboardstyle.h"

#include <MNamespace>
#include <MButton>
#include <QKeySequence>
#include <QGraphicsLinearLayout>
#include <QDebug>
#include <MSceneManager>
#include <mplainwindow.h>
#include <mreactionmap.h>
#include <MScalableImage>

namespace
{
    //!object name for toolbar buttons
    const QString ObjectNameToolbarButtons("VirtualKeyboardToolbarButton");
    const QString ObjectNameCloseButton("VirtualKeyboardCloseButton");
    const QString ObjectNameToolbar("MImToolbar");
    const QString ObjectNameToolbarLeft("VirtualKeyboardToolbarLeft");
    const QString ObjectNameToolbarRight("VirtualKeyboardToolbarRight");
    const QString IconNameCloseButton("icon-m-input-methods-close");
};

MImToolbar::MImToolbar(MVirtualKeyboardStyleContainer &style, QGraphicsWidget *parent)
    : MWidget(parent),
      toolbarMgr(ToolbarManager::instance()),
      textSelected(false),
      copyPaste(new MButton),
      copyPasteStatus(InputMethodNoCopyPaste),
      leftBar(true, this),
      rightBar(true, this),
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

    connect(&toolbarMgr, SIGNAL(buttonClicked(ToolbarWidget)), this, SLOT(handleButtonClick(ToolbarWidget)));
}

MImToolbar::~MImToolbar()
{
    delete copyPaste;
    copyPaste = 0;
}

void MImToolbar::setupLayout()
{
    QGraphicsLinearLayout *mainLayout = new QGraphicsLinearLayout(Qt::Horizontal, this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Add the left and right side WidgetBar widgets with a stretch item in between.
    mainLayout->addItem(&leftBar);
    mainLayout->addStretch();
    mainLayout->addItem(&rightBar);

    mainLayout->setAlignment(&leftBar, Qt::AlignBottom);
    mainLayout->setAlignment(&rightBar, Qt::AlignBottom);

    resize(geometry().width(), layout()->preferredHeight());
}

void MImToolbar::loadDefaultButtons()
{
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

void MImToolbar::handleButtonClick(const ToolbarWidget &button)
{
    if (button.type() != ToolbarWidget::Button)
        return;

    foreach(const ToolbarWidget::Action *action, button.actions) {
        switch (action->type) {
        case ToolbarWidget::SendKeySequence:
            sendKeySequence(action->keys);
            break;
        case ToolbarWidget::SendString:
            sendStringRequest(action->text);
            break;
        case ToolbarWidget::SendCommand:
            //TODO:not support yet
            break;
        case ToolbarWidget::Copy:
            emit copyPasteRequest(InputMethodCopy);
            break;
        case ToolbarWidget::Paste:
            emit copyPasteRequest(InputMethodPaste);
            break;
        case ToolbarWidget::ShowGroup:
            showGroup(action->group);
            break;
        case ToolbarWidget::HideGroup:
            hideGroup(action->group);
            break;
        case ToolbarWidget::Unknown:
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
    //set widget's visibility according showOn and hideOn premiss and current selection status
    foreach(ToolbarWidget *w, toolbarMgr.widgetList()) {
        if ((textSelected && w->hideOn != ToolbarWidget::WhenSelectingText)
                || (w->showOn == ToolbarWidget::Always)) {
            w->setVisible(true);
        } else {
            w->setVisible(false);
        }
    }

    // Update widgets according to toolbar models.
    updateWidgets();
}

void MImToolbar::loadCustomWidgets(Qt::Alignment align)
{
    qDebug() << __PRETTY_FUNCTION__ << align;
    //the widgets gotten from toolbarMgr are already ordered acording their priority with alignment.
    QList<ToolbarWidget *> widgets = toolbarMgr.widgetList(align);
    //show widgets according their status and priority
    int widgetCount = 0;
    foreach(ToolbarWidget *toolbarWidget, widgets) {
        MWidget *widget = toolbarMgr.widget(toolbarWidget->name());
        if (!widget)
            continue;
        if (toolbarWidget->isVisible()) {
            widget->setVisible(true);
            //if widget is Visible, then insert it to the right position
            insertItem(widgetCount, widget, align);
            ++widgetCount;
        } else {
            widget->setVisible(false);
            removeItem(widget);
        }
    }
}

void MImToolbar::unloadCustomWidgets(Qt::Alignment align)
{
    QList<ToolbarWidget *> widgets = toolbarMgr.widgetList(align);
    foreach(ToolbarWidget *toolbarWidget, widgets) {
        MWidget *widget = toolbarMgr.widget(toolbarWidget->name());
        if (!widget)
            continue;
        widget->setVisible(false);
        removeItem(widget);
    }
}

void MImToolbar::updateWidgets(bool customWidgetsChanged)
{
    if (customWidgetsChanged) {
        loadCustomWidgets(Qt::AlignLeft);
        loadCustomWidgets(Qt::AlignRight);
    }

    if (isVisible()) {
        layout()->invalidate();
    }

    emit regionUpdated();
}

void MImToolbar::showGroup(const QString &group)
{
    bool changed = false;
    foreach(ToolbarWidget *w, toolbarMgr.widgetList()) {
        if (w->group == group && !(w->isVisible())) {
            w->setVisible(true);
            changed = true;
        }
    }

    if (changed) {
        updateWidgets();
    }
}

void MImToolbar::hideGroup(const QString &group)
{
    bool changed = false;
    foreach(ToolbarWidget *w, toolbarMgr.widgetList()) {
        if (w->group == group && w->isVisible()) {
            w->setVisible(false);
            changed = true;
        }
    }
    if (changed) {
        updateWidgets(true);
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

void MImToolbar::showToolbarWidget(qlonglong id)
{
    qDebug() << __PRETTY_FUNCTION__ << id;
    if (id != toolbarMgr.currentToolbar()) {
        unloadCustomWidgets(Qt::AlignLeft);
        unloadCustomWidgets(Qt::AlignRight);
    }
    ToolbarManager::instance().loadToolbar(id);
    if (isVisible())
        updateVisibility();
}

void MImToolbar::hideToolbarWidget()
{
    qDebug() << __PRETTY_FUNCTION__;
    unloadCustomWidgets(Qt::AlignLeft);
    unloadCustomWidgets(Qt::AlignRight);
    ToolbarManager::instance().reset();
    if (isVisible())
        updateVisibility();
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
    // copy button is the most right button in toolbar
    const int buttonIndex = rightBar.count();
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
        updateWidgets(false);
    }
    qDebug() << __PRETTY_FUNCTION__ << copyPaste->isVisible();
}

void MImToolbar::insertItem(const int index, MWidget *widget, Qt::Alignment align)
{
    Q_ASSERT((align == Qt::AlignLeft) || (align == Qt::AlignRight));
    WidgetBar *sidebar = (align == Qt::AlignLeft) ? &leftBar : &rightBar;
    if (!sidebar->contains(widget)) {
        sidebar->insert(index, widget);
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

void MImToolbar::removeItem(MWidget *widget)
{
    leftBar.remove(widget);
    rightBar.remove(widget);

    if (leftBar.count() == 0) {
        leftBar.hide();
    }
    if (rightBar.count() == 0) {
        rightBar.hide();
    }

    layout()->invalidate();
    layout()->activate();
}

void MImToolbar::drawReactiveAreas(MReactionMap *reactionMap, QGraphicsView *view)
{
    // TODO: support for translucent keyboard
    reactionMap->setTransform(this, view);
    reactionMap->setInactiveDrawingValue();
    reactionMap->fillRectangle(rect());

    // Draw all widgets geometries.
    reactionMap->setReactiveDrawingValue();

    for (int j = 0; j < 2; ++j) {
        WidgetBar *sidebar = ((j == 0) ? &leftBar : &rightBar);
        if (!sidebar->isVisible()) {
            continue;
        }
        reactionMap->setTransform(sidebar, view);

        for (int i = 0; i < sidebar->count(); ++i) {
            reactionMap->fillRectangle(sidebar->widgetAt(i)->geometry());
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
    MReactionMap *reactionMap = MReactionMap::instance(view);
    if (!reactionMap)
        return;

    reactionMap->setTransform(this, view);
    reactionMap->setInactiveDrawingValue();
    reactionMap->fillRectangle(leftBar.geometry());
    reactionMap->fillRectangle(rightBar.geometry());
}
