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


#ifndef __MGCONFITEM_STUB_H__
#define __MGCONFITEM_STUB_H__

#include <mgconfitem.h>
#include <QDebug>

#include "fakegconf.h"

/**
 * MGConfItem stub class.
 * To fake MGConfItem operations, derive from this class
 * and implement the methods you want to fake. Instantiate your
 * derived stub class and assign it to gMGConfItemStub
 * global variable.
 */
class MGConfItemStub
{
public:
    virtual void mGConfItemConstructor(const MGConfItem *instance, const QString &key, QObject *parent = 0);
    virtual void mGConfItemDestructor(const MGConfItem *instance);
    virtual QString key(const MGConfItem *instance);
    virtual QVariant value(const MGConfItem *instance);
    virtual QVariant value(const MGConfItem *instance, const QVariant &def);
    virtual void set(const MGConfItem *instance, const QVariant &val);
    virtual QList<QString> listDirs(const MGConfItem *instance);
    virtual QList<QString> listEntries(const MGConfItem *instance);

protected:
    QMap<const MGConfItem *, QString> instanceKeys; // This map links MGConfItem instance to its present key.
    FakeGConf fakeGConf; // This is the in-memory storage for settings.
};

void MGConfItemStub::mGConfItemConstructor(const MGConfItem *instance, const QString &key, QObject *)
{
    if (!key.isEmpty()) {
        FakeGConfItem *fakeItem = fakeGConf.initKey(key);
        QObject::connect(fakeItem, SIGNAL(valueChanged()), instance, SIGNAL(valueChanged()));

        instanceKeys[instance] = key;
    }
}

void MGConfItemStub::mGConfItemDestructor(const MGConfItem *instance)
{
    instanceKeys.remove(instance);
}

QString MGConfItemStub::key(const MGConfItem *instance)
{
    return instanceKeys[instance];
}

QVariant MGConfItemStub::value(const MGConfItem *instance)
{
    return fakeGConf.value(instanceKeys[instance]);
}

QVariant MGConfItemStub::value(const MGConfItem *instance, const QVariant &def)
{
    QVariant val = fakeGConf.value(instanceKeys[instance]);
    if (val.isNull())
        val = def;
    return val;
}

void MGConfItemStub::set(const MGConfItem *instance, const QVariant &value)
{
    fakeGConf.setValue(instanceKeys[instance], value);
}

QList<QString> MGConfItemStub::listDirs(const MGConfItem *instance)
{
    return fakeGConf.listDirs(instanceKeys[instance]);
}

QList<QString> MGConfItemStub::listEntries(const MGConfItem *instance)
{
    return fakeGConf.listEntries(instanceKeys[instance]);
}


/**
 * This is the stub class instance used by the system. If you want to alter behaviour,
 * derive your stub class from MGConfItemStub, implement the methods you want to
 * fake, create an instance of your stub class and assign the instance into this global variable.
 */
// this dynamic alloc for gMGConfItemStub will cause memory leak. But it is accaptable for
// unit test, because it is just a small memory leak. And this can avoid core dump if there are
// some static MGConfItem object declared by application.
MGConfItemStub *gMGConfItemStub = new MGConfItemStub;

/**
 * These are the proxy method implementations of MGConfItem. They will
 * call the stub object methods of the gMGConfItemStub.
 */

MGConfItem::MGConfItem(const QString &key, QObject *parent)
{
    gMGConfItemStub->mGConfItemConstructor(this, key, parent);
}

MGConfItem::~MGConfItem()
{
    gMGConfItemStub->mGConfItemDestructor(this);
}

QString MGConfItem::key() const
{
    return gMGConfItemStub->key(this);
}

QVariant MGConfItem::value() const
{
    return gMGConfItemStub->value(this);
}

QVariant MGConfItem::value(const QVariant &def) const
{
    return gMGConfItemStub->value(this, def);
}

void MGConfItem::set(const QVariant &val)
{
    gMGConfItemStub->set(this, val);
}

QList<QString> MGConfItem::listDirs() const
{
    return gMGConfItemStub->listDirs(this);
}

QList<QString> MGConfItem::listEntries() const
{
    return gMGConfItemStub->listEntries(this);
}

#endif //__MGCONFITEM_STUB_H__
