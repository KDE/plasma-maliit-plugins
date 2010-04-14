/* * This file is part of dui-keyboard *
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



#include "toolbarmanager.h"
#include "toolbardata.h"
#include "duiimtoolbar.h"
#include <DuiGConfItem>
#include <DuiButton>
#include <DuiLocale>
#include <QDebug>

namespace
{
    const QString ObjectNameToolbarButtons("VirtualKeyboardToolbarButton");
    const int MaximumToolbarCount = 10;
}

const int ToolbarManager::buttonNameDataKey(1000);

ToolbarManager::ToolbarManager(DuiImToolbar *parent)
    : imToolbar(parent),
      current(0)
{
}

ToolbarManager::~ToolbarManager()
{
    qDeleteAll(toolbars);
    toolbars.clear();

    qDeleteAll(toolbarButtonPool);
    toolbarButtonPool.clear();
}

int ToolbarManager::buttonCount() const
{
    if (current != 0)
        return current->buttons.count();
    else
        return 0;
}

QList<ToolbarButton *> ToolbarManager::buttonList() const
{
    QList<ToolbarButton *> list;
    if (current != 0)
        list = current->buttons;

    return list;
}

QList<ToolbarButton *> ToolbarManager::buttonList(Qt::Alignment align) const
{
    QList<ToolbarButton *> list;
    if (current != 0) {
        foreach(ToolbarButton * b, current->buttons) {
            if (b->alignment == align) {
                //sort according priority
                bool inserted = false;
                for (int i = 0; i < list.count(); ++i) {
                    if (((align == Qt::AlignLeft) && (list[i]->priority < b->priority))
                            || ((align == Qt::AlignRight) && (list[i]->priority > b->priority)))
                        continue;
                    else {
                        inserted = true;
                        list.insert(i, b);
                        break;
                    }
                }
                if (!inserted) {
                    list.append(b);
                    continue;
                }
            }
        }
    }
    return list;
}

ToolbarButton *ToolbarManager::toolbarButton(const QString &name) const
{
    foreach(ToolbarButton * tb, current->buttons)
    if (tb->name == name)
        return tb;
    return 0;
}

ToolbarButton *ToolbarManager::toolbarButton(const DuiButton *b) const
{
    if (b)
        return toolbarButton(b->data(buttonNameDataKey).toString());
    return 0;
}

DuiButton *ToolbarManager::button(const QString &name) const
{
    DuiButton *b = 0;
    for (int i = 0; i < toolbarButtonPool.count(); i++) {
        if (toolbarButtonPool[i]->data(buttonNameDataKey).toString() == name) {
            b = toolbarButtonPool[i];
            break;
        }
    }
    return b;
}

QStringList ToolbarManager::toolbarList() const
{
    QStringList names;
    foreach(const ToolbarData * t, toolbars)
    names << t->fileName();
    return names;
}

QString ToolbarManager::currentToolbar() const
{
    if (current != 0)
        return current->fileName();
    return QString();
}

bool ToolbarManager::loadToolbar(const QString &name)
{
    // sanity tests
    if (name.isEmpty()) {
        return false;
    }

    //already load
    if ((current != 0) && (current->equal(name)))
        return true;

    current = findToolbar(name);
    if (!current) {
        current = createToolbar(name);
    } else {
        //move current (toolbar) to the beginning of toolbars.
        toolbars.move(toolbars.indexOf(current), 0);
    }

    if (current != 0) {
        loadToolbarButtons();
        return true;
    }
    //can't find
    reset();
    return false;
}

ToolbarData *ToolbarManager::findToolbar(const QString &name)
{
    foreach (ToolbarData *t, toolbars) {
        if (t->equal(name))
            return t;
    }
    return 0;
}

ToolbarData *ToolbarManager::createToolbar(const QString &name)
{
    // load a toolbar
    ToolbarData *toolbar = new ToolbarData;
    const bool loaded = toolbar->loadNokiaToolbarXml(name);

    if (!loaded) {
        qWarning() << "ToolbarsManager: toolbar load error: "
                   << name;
        delete toolbar;
        toolbar = 0;
    } else {
        // if cached toolbars reach MaximumToolbarCount, then remove the most rarely used toolbar.
        if (toolbars.count() >= MaximumToolbarCount) {
            // the last toolbar is the most rarely used one.
            delete toolbars.takeLast();
        }
        toolbars.prepend(toolbar);
    }

    return toolbar;
}

void ToolbarManager::reset()
{
    current = 0;
}

void ToolbarManager::loadToolbarButtons()
{
    qDebug() << __PRETTY_FUNCTION__;
    resetButtonPool();
    foreach (const ToolbarButton *b, buttonList()) {
        createButton(b);
    }
}

void ToolbarManager::createButton(const ToolbarButton *tb)
{
    if (!tb)
        return;

    DuiButton *b = button(tb->name);
    //because name is the unique id, so if there is already created, then just use it
    if (!b) {
        //find the first unused pool item, or create a new one
        for (int i = 0; i < toolbarButtonPool.count(); i++) {
            if (toolbarButtonPool[i]->data(buttonNameDataKey).toString().isEmpty()) {
                b = toolbarButtonPool[i];
                break;
            }
        }
        if (!b) {
            b = new DuiButton();
            b->setObjectName(ObjectNameToolbarButtons);
            toolbarButtonPool.append(b);
        }
    }
    b->setData(buttonNameDataKey, tb->name);
    if (!tb->textId.isEmpty())
        b->setText(qtTrId(tb->textId.toUtf8().data()));
    else
        b->setText(tb->text);
    b->setIconID(tb->icon);
    // Common signals
    connect(b, SIGNAL(clicked()), imToolbar, SLOT(handleButtonClick()));
}

void ToolbarManager::resetButtonPool()
{
    for (int i = 0; i < toolbarButtonPool.count(); i++) {
        toolbarButtonPool[i]->setData(buttonNameDataKey, "");
        toolbarButtonPool[i]->setVisible(false);
        toolbarButtonPool[i]->disconnect();
    }
}
