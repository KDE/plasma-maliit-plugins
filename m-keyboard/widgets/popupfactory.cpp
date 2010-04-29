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




#include "popupfactory.h"
#include "popupbase.h"
#include "popupplugin.h"

#include <QDir>
#include <QPluginLoader>
#include <QDebug>


namespace
{
    const char * const PluginDir = "/usr/lib/meego-im-plugins/meego-keyboard/";
}


// dummy popup to be used if no plugin is found
class DummyPopup: public PopupBase
{
public:
    DummyPopup(const MVirtualKeyboardStyleContainer &styleContainer);
    ~DummyPopup();

    //! \reimp
    virtual void updatePos(QPointF keyPos, QPoint screenPos, const QSize &keySize);
    virtual void hidePopup();
    virtual void showPopup();
    virtual bool isPopupVisible() const;
    virtual void setFingerPos(const QPointF &pos);
    //! \reimp_end

private:
    bool visible;
};



PopupFactory *PopupFactory::instance()
{
    static PopupFactory instance;
    return &instance;
}


PopupBase *
PopupFactory::createPopup(const MVirtualKeyboardStyleContainer &styleContainer,
                          QGraphicsItem *parent) const
{
    // if plugin present, use it to create popup, otherwise return dummy popup
    PopupBase *popup;
    if (plugin) {
        popup = plugin->createPopup(styleContainer, parent);
    } else {
        popup = new DummyPopup(styleContainer);
    }

    return popup;
}


PopupFactory::PopupFactory()
    : plugin(0)
{
    // setup pluginloader
    QDir pluginsDir(PluginDir);
    QStringList filters;
    filters << "lib*.so";
    pluginsDir.setNameFilters(filters);
    QStringList fileList = pluginsDir.entryList();

    if (!fileList.isEmpty()) {
        // just load the first file
        QString fileName = fileList.at(0);
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *pluginInstance = loader.instance();

        plugin = qobject_cast<PopupPlugin *>(pluginInstance);
        
        if (!plugin) {
            qDebug() << "Error loading mvkb sub-plugin:" << loader.errorString();
        }
    }
}


PopupFactory::~PopupFactory()
{
}


/////////////////////////////////
// Dummy popup implementation
DummyPopup::DummyPopup(const MVirtualKeyboardStyleContainer &styleContainer)
    : PopupBase(styleContainer),
      visible(false)
{
    // nothing
}

DummyPopup::~DummyPopup()
{
    // nothing
}

void DummyPopup::updatePos(QPointF /*keyPos*/, QPoint /*screenPos*/, const QSize &/*keySize*/)
{
    // nothing
}

void DummyPopup::hidePopup()
{
    visible = false;
}


void DummyPopup::showPopup()
{
    visible = true;
}


bool DummyPopup::isPopupVisible() const
{
    return visible;
}

void DummyPopup::setFingerPos(const QPointF &pos)
{
    Q_UNUSED(pos)
}
