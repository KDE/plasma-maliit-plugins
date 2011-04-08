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
class MockPopup: public PopupBase
{
public:
    explicit MockPopup(MImAbstractKeyArea *mainArea)
        : PopupBase(mainArea),
          visible(false)
    {}

    virtual ~MockPopup()
    {}

    //! \reimp
    virtual void updatePos(const QPointF &,
                           const QPoint &,
                           const QSize &)
    {}

    virtual void cancel()
    {
        visible = false;
    }

    virtual void handleKeyPressedOnMainArea(MImAbstractKey *,
                                            const QString &,
                                            bool)
    {
        visible = true;
    }

    virtual void handleLongKeyPressedOnMainArea(MImAbstractKey *,
                                                const QString &,
                                                bool)
    {
        visible = true;
    }

    virtual bool isVisible() const
    {
        return visible;
    }

    virtual void setVisible(bool newVisible)
    {
        visible = newVisible;
    }
    //! \reimp_end
private:
    bool visible;
};

PopupFactory *PopupFactory::instance()
{
    static PopupFactory instance;
    return &instance;
}

PopupBase *PopupFactory::createPopup(MImAbstractKeyArea *mainArea) const
{
#ifdef UNIT_TEST
    return new MockPopup(mainArea);
#else
    return (plugin ? plugin->createPopup(mainArea)
                   : new MockPopup(mainArea));
#endif
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
{}
