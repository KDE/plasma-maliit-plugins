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
ToolbarManager *ToolbarManager::toolbarMgrInstance = 0;

ToolbarManager::ToolbarManager()
    : current(cachedToolbars.end())
{
}

ToolbarManager::~ToolbarManager()
{
    qDeleteAll(cachedToolbars.values());
    cachedToolbars.clear();
    cachedToolbarIds.clear();

    qDeleteAll(toolbarWidgetPool);
    toolbarWidgetPool.clear();
}

void ToolbarManager::createInstance()
{
    Q_ASSERT(!toolbarMgrInstance);
    if (!toolbarMgrInstance) {
        toolbarMgrInstance = new ToolbarManager;
    }
}

void ToolbarManager::destroyInstance()
{
    Q_ASSERT(toolbarMgrInstance);
    delete toolbarMgrInstance;
    toolbarMgrInstance = 0;
}

int ToolbarManager::widgetCount() const
{
    if (current != cachedToolbars.end())
        return current.value()->widgets.count();
    else
        return 0;
}

QList<ToolbarWidget *> ToolbarManager::widgetList() const
{
    QList<ToolbarWidget *> list;
    if (current != cachedToolbars.end())
        list = current.value()->widgets;

    return list;
}

QList<ToolbarWidget *> ToolbarManager::widgetList(Qt::Alignment align) const
{
    QList<ToolbarWidget *> list;
    if (current != cachedToolbars.end()) {
        foreach(ToolbarWidget *tw, current.value()->widgets) {
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

ToolbarWidget *ToolbarManager::toolbarWidget(qlonglong id, const QString &name) const
{
    if (current != cachedToolbars.end() && current.key() == id) {
        return toolbarWidget(name);
    } else {
        CachedToolbarContainer::const_iterator iterator = cachedToolbars.find(id);
        if (iterator != cachedToolbars.end()) {
            foreach(ToolbarWidget *tw, iterator.value()->widgets) {
                if (tw->name() == name)
                    return tw;
            }
        }
    }
    return 0;
}

ToolbarWidget *ToolbarManager::toolbarWidget(const QString &name) const
{
    if (current != cachedToolbars.end()) {
        foreach(ToolbarWidget *tw, current.value()->widgets) {
            if (tw->name() == name)
                return tw;
        }
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
        if (toolbarWidgetPool[i] && toolbarWidgetPool[i]->data(widgetNameDataKey).toString() == name) {
            w = toolbarWidgetPool[i];
            break;
        }
    }
    return w;
}

QList<qlonglong> ToolbarManager::toolbarList() const
{
    return cachedToolbars.keys();
}

qlonglong ToolbarManager::currentToolbar() const
{
    if (current != cachedToolbars.end())
        return current.key();
    return -1;
}

bool ToolbarManager::loadToolbar(qlonglong id)
{
    // sanity tests
    if (!toolbars.contains(id)) {
        qWarning() << "ToolbarsManager: toolbar load error: " << id;
        reset();
        return false;
    }

    //already load
    if ((current != cachedToolbars.end()) && (current.key() == id) && validateWidgetPool()) {
        return true;
    }

    current = cachedToolbars.find(id);
    if (current == cachedToolbars.end()) {
        ToolbarData * toolbarData = createToolbar(toolbars.value(id));
        if (toolbarData) {
            // if cached toolbars reach MaximumToolbarCount, then remove the most rarely used toolbar.
            if (cachedToolbarIds.count() >= MaximumToolbarCount) {
                // the last toolbar is the most rarely used one.
                delete cachedToolbars.take(cachedToolbarIds.takeLast());
            }
            current = cachedToolbars.insert(id, toolbarData);
            cachedToolbarIds.prepend(id);
        }
    } else {
        //move current (toolbar) to the beginning of cahced toolbars.
        cachedToolbarIds.move(cachedToolbarIds.indexOf(current.key()), 0);
    }

    if (current != cachedToolbars.end()) {
        loadToolbarWidgets();
        return true;
    } else {
        //can't find
        return false;
    }
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
    }

    return toolbar;
}

void ToolbarManager::reset()
{
    resetWidgetPool();
    current = cachedToolbars.end();
}

void ToolbarManager::loadToolbarWidgets()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!current.value()->toolbarPixmapDirectory.isEmpty()) {
        MTheme::instance()->addPixmapDirectory(current.value()->toolbarPixmapDirectory);
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

    QPointer<MWidget> w = widget(tw->name());
    //because name is the unique id, so if there is already created, then just use it
    if (!w) {
        //find the first unused pool item, or create a new one
        for (int i = 0; i < toolbarWidgetPool.count(); i++) {
            if (toolbarWidgetPool[i] && toolbarWidgetPool[i]->data(widgetNameDataKey).toString().isEmpty()
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
            connect(b, SIGNAL(clicked()), this, SLOT(handleButtonClick()));
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
    validateWidgetPool();
    for (int i = 0; i < toolbarWidgetPool.count(); i++) {
        toolbarWidgetPool[i]->setData(widgetNameDataKey, "");
        toolbarWidgetPool[i]->setVisible(false);
        toolbarWidgetPool[i]->disconnect();
    }
}

bool ToolbarManager::validateWidgetPool()
{
    // Widgets in the widget pool could be destroyed outside by MImToolbar.
    // But QPointer can safely promise the pointers are already set to 0,
    // so remove all null widget from pool.
    bool valid = (toolbarWidgetPool.removeAll(0) > 0) ? false : true;
    return valid;
}

void ToolbarManager::handleButtonClick()
{
    const MButton *button = qobject_cast<MButton *>(this->sender());
    Q_ASSERT(button);

    const ToolbarWidget *tw = toolbarWidget(button);
    if (!tw)
        return;

    emit buttonClicked(*tw);
}

void ToolbarManager::registerToolbar(qlonglong id, const QString &fileName)
{
    qDebug() << __PRETTY_FUNCTION__;
    if ((id <= 0) || fileName.isEmpty() || toolbars.contains(id))
        return;

    toolbars.insert(id, fileName);

    ToolbarData * toolbarData = createToolbar(fileName);
    if (toolbarData) {
        // if cached toolbars reach MaximumToolbarCount, then remove the most rarely used toolbar.
        if (cachedToolbarIds.count() >= MaximumToolbarCount) {
            // the last toolbar is the most rarely used one.
            delete cachedToolbars.take(cachedToolbarIds.takeLast());
        }
        cachedToolbars.insert(id, toolbarData);
        cachedToolbarIds.prepend(id);
    }
}

void ToolbarManager::unregisterToolbar(qlonglong id)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!toolbars.contains(id))
        return;

    if (cachedToolbars.contains(id)) {
        if (id == currentToolbar()) {
            reset();
        }
        delete cachedToolbars.take(id);
        cachedToolbarIds.removeOne(id);
    }
    toolbars.remove(id);
}

void ToolbarManager::setToolbarItemAttribute(qlonglong id, const QString &item, const QString &attribute, const QVariant &value)
{
    qDebug() << __PRETTY_FUNCTION__;
    if ((id <= 0) || item.isEmpty() || attribute.isEmpty() || value.isNull())
        return;

    ToolbarWidget *tw = toolbarWidget(id, item);
    if (tw) {
        switch (tw->type()) {
        case ToolbarWidget::Button: {
            MButton *button = qobject_cast<MButton *>(widget(item));
            if (!button)
                return;
            if (attribute == "icon") {
                button->setIconID(value.toString());
            } else if (attribute == "text") {
                button->setText(value.toString());
            } else if (attribute == "textid") {
                button->setText(qtTrId(value.toString().toUtf8().data()));
            } else if (attribute == "pressed" && tw->toggle) {
                button->setChecked(value.toBool());
            }
            break;
        }
        case ToolbarWidget::Label: {
            MLabel *label = qobject_cast<MLabel *>(widget(item));
            if (!label)
                return;
            if (attribute == "text") {
                label->setText(value.toString());
            } else if (attribute == "textid") {
                label->setText(qtTrId(value.toString().toUtf8().data()));
            }
            break;
        }
        default:
            break;
        }
    }
}

