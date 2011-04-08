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



#include "fakegconf.h"
#include <QDebug>

FakeGConf::FakeGConf() :
    root(NULL, "/")
{
}

FakeGConf::~FakeGConf()
{
}

FakeGConfItem *FakeGConf::initKey(const QString &key)
{
    QStringList path = key.split("/", QString::SkipEmptyParts);
    FakeGConfItem *item = &root;
    foreach(QString keyName, path) {
        FakeGConfItem *child = item->findChild(keyName);
        if (child == NULL) {
            child = new FakeGConfItem(item, keyName);
        }
        item = child;
    }
    return item;
}

void FakeGConf::setValue(const QString &key, const QVariant &value)
{
    initKey(key)->setValue(value);
}

QVariant FakeGConf::value(const QString &key)
{
    return initKey(key)->value();
}

QStringList FakeGConf::listDirs(const QString &key)
{
    QList<FakeGConfItem *> children = initKey(key)->children();
    QStringList dirs;

    // Get every child that has children, i.e. it is called "directory"
    foreach(FakeGConfItem * child, children) {
        if (child->hasChildren())
            dirs << child->name();
    }
    return dirs;
}

QStringList FakeGConf::listEntries(const QString &key)
{
    QList<FakeGConfItem *> children = initKey(key)->children();
    QStringList entries;

    // Get every child that has value
    foreach(FakeGConfItem * child, children) {
        if (child->hasValueSet())
            entries << child->name();
    }
    return entries;
}

FakeGConfItem::FakeGConfItem(FakeGConfItem *parent, const QString &name) :
    parent(parent),
    keyName(name),
    keyValue(QVariant())
{
    if (parent)
        parent->childList.append(this);
}

FakeGConfItem::~FakeGConfItem()
{
    qDeleteAll(childList);
    childList.clear();
}

FakeGConfItem *FakeGConfItem::findChild(const QString &childName)
{
    FakeGConfItem *result = 0;
    foreach(FakeGConfItem * child, childList) {
        if (child->keyName == childName) {
            result = child;
            break;
        }
    }
    return result;
}

QList<FakeGConfItem *> FakeGConfItem::children()
{
    return childList;
}

bool FakeGConfItem::hasChildren()
{
    return (childList.count() > 0);
}

bool FakeGConfItem::hasValueSet()
{
    return !keyValue.isNull();
}

QString FakeGConfItem::name()
{
    return keyName;
}

QVariant FakeGConfItem::value()
{
    return keyValue;
}

void FakeGConfItem::setValue(const QVariant &value)
{
    bool changed = false;

    if (keyValue != value)
        changed = true;

    keyValue = value;

    if (changed)
        emit valueChanged();
}
