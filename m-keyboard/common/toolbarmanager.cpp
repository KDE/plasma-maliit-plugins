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



#include "toolbarmanager.h"
#include "toolbardata.h"
#include "mimtoolbar.h"
#include <MGConfItem>
#include <MTheme>
#include <MButton>
#include <MLabel>
#include <MLocale>
#include <QDebug>

namespace
{
    const QString ObjectNameToolbarButton("VirtualKeyboardToolbarButton");
    const QString ObjectNameToolbarLabel("VirtualKeyboardToolbarLabel");
    const int MaximumToolbarCount = 10;
}

const int ToolbarManager::widgetNameDataKey(1000);
const int ToolbarManager::widgetTypeDataKey(1001);

ToolbarManager::ToolbarManager(MImToolbar *parent)
    : imToolbar(parent),
      current(0)
{
}

ToolbarManager::~ToolbarManager()
{
    qDeleteAll(toolbars);
    toolbars.clear();

    qDeleteAll(toolbarWidgetPool);
    toolbarWidgetPool.clear();
}

int ToolbarManager::widgetCount() const
{
    if (current != 0)
        return current->widgets.count();
    else
        return 0;
}

QList<ToolbarWidget *> ToolbarManager::widgetList() const
{
    QList<ToolbarWidget *> list;
    if (current != 0)
        list = current->widgets;

    return list;
}

QList<ToolbarWidget *> ToolbarManager::widgetList(Qt::Alignment align) const
{
    QList<ToolbarWidget *> list;
    if (current != 0) {
        foreach(ToolbarWidget *tw, current->widgets) {
            if (tw->alignment == align) {
                //sort according priority
                bool inserted = false;
                for (int i = 0; i < list.count(); ++i) {
                    if (((align == Qt::AlignLeft) && (list[i]->priority < tw->priority))
                            || ((align == Qt::AlignRight) && (list[i]->priority > tw->priority)))
                        continue;
                    else {
                        inserted = true;
                        list.insert(i, tw);
                        break;
                    }
                }
                if (!inserted) {
                    list.append(tw);
                    continue;
                }
            }
        }
    }
    return list;
}

ToolbarWidget *ToolbarManager::toolbarWidget(const QString &name) const
{
    foreach(ToolbarWidget *tw, current->widgets) {
        if (tw->name() == name)
            return tw;
    }
    return 0;
}

ToolbarWidget *ToolbarManager::toolbarWidget(const MWidget *w) const
{
    if (w)
        return toolbarWidget(w->data(widgetNameDataKey).toString());
    return 0;
}

MWidget *ToolbarManager::widget(const QString &name) const
{
    MWidget *w = 0;
    for (int i = 0; i < toolbarWidgetPool.count(); i++) {
        if (toolbarWidgetPool[i]->data(widgetNameDataKey).toString() == name) {
            w = toolbarWidgetPool[i];
            break;
        }
    }
    return w;
}

QStringList ToolbarManager::toolbarList() const
{
    QStringList names;
    foreach(const ToolbarData *t, toolbars)
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
        loadToolbarWidgets();
        return true;
    }
    //can't find
    reset();
    return false;
}

ToolbarData *ToolbarManager::findToolbar(const QString &name)
{
    foreach(ToolbarData *t, toolbars) {
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

void ToolbarManager::loadToolbarWidgets()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!current->toolbarPixmapDirectory.isEmpty()) {
        MTheme::instance()->addPixmapDirectory(current->toolbarPixmapDirectory);
    }
    resetWidgetPool();
    foreach(const ToolbarWidget *tw, widgetList()) {
        createWidget(tw);
    }
}

void ToolbarManager::createWidget(const ToolbarWidget *tw)
{
    if (!tw)
        return;

    MWidget *w = widget(tw->name());
    //because name is the unique id, so if there is already created, then just use it
    if (!w) {
        //find the first unused pool item, or create a new one
        for (int i = 0; i < toolbarWidgetPool.count(); i++) {
            if (toolbarWidgetPool[i]->data(widgetNameDataKey).toString().isEmpty()
                && toolbarWidgetPool[i]->data(widgetTypeDataKey).toInt() == tw->type()) {
                w = toolbarWidgetPool[i];
                break;
            }
        }
        if (!w) {
            switch (tw->type()) {
            case ToolbarWidget::Button:
                w = new MButton();
                w->setObjectName(ObjectNameToolbarButton);
                break;
            case ToolbarWidget::Label:
                w = new MLabel();
                w->setObjectName(ObjectNameToolbarLabel);
                break;
            default:
                //unknown widget type.
                return;
            }
            toolbarWidgetPool.append(w);
        }
    }
    w->setData(widgetNameDataKey, tw->name());
    w->setData(widgetTypeDataKey, tw->type());

    switch (tw->type()) {
    case ToolbarWidget::Button: {
        MButton *b = qobject_cast<MButton*>(w);
        if (b) {
            if (!tw->textId.isEmpty()) {
                b->setText(qtTrId(tw->textId.toUtf8().data()));
            } else {
                b->setText(tw->text);
            }
            b->setIconID(tw->icon);
            b->setCheckable(tw->toggle);
            if (tw->toggle) {
                b->setChecked(tw->pressed);
            }
            connect(b, SIGNAL(clicked()), imToolbar, SLOT(handleButtonClick()));
        }
        break;
    }
    case ToolbarWidget::Label: {
        MLabel *l = qobject_cast<MLabel*>(w);
        if (!tw->textId.isEmpty()) {
            l->setText(qtTrId(tw->textId.toUtf8().data()));
        } else {
            l->setText(tw->text);
        }
        break;
    }
    default:
        break;
    }
}

void ToolbarManager::resetWidgetPool()
{
    for (int i = 0; i < toolbarWidgetPool.count(); i++) {
        toolbarWidgetPool[i]->setData(widgetNameDataKey, "");
        toolbarWidgetPool[i]->setVisible(false);
        toolbarWidgetPool[i]->disconnect();
    }
}
