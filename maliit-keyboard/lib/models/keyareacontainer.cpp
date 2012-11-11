/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2012 Openismus GmbH. All rights reserved.
 *
 * Contact: maliit-discuss@lists.maliit.org
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
 *
 */

#include "keyareacontainer.h"
#include "keyarea.h"
#include "key.h"

namespace MaliitKeyboard {
namespace Model {


class KeyAreaContainerPrivate
{
public:
    KeyArea key_area;

    explicit KeyAreaContainerPrivate();
};


KeyAreaContainerPrivate::KeyAreaContainerPrivate()
    : key_area()
{}


KeyAreaContainer::KeyAreaContainer(QObject *parent)
    : QAbstractListModel(parent)
    , d_ptr(new KeyAreaContainerPrivate)
{
    // Model roles are used as variables in QML, hence the under_score naming
    // convention:
    QHash<int, QByteArray> roles;
    roles[RoleKeyRect] = "key_rect";
    roles[RoleKeyBackground] = "key_background";
    roles[RoleKeyText] = "key_text";
    setRoleNames(roles);
}


KeyAreaContainer::~KeyAreaContainer()
{}


void KeyAreaContainer::setKeyArea(const KeyArea &area)
{
    beginResetModel();

    Q_D(KeyAreaContainer);
    const bool geometry_changed(d->key_area.rect() != area.rect());

    d->key_area = area;

    if (geometry_changed) {
        Q_EMIT widthChanged(width());
        Q_EMIT heightChanged(height());
    }

    endResetModel();
}


KeyArea KeyAreaContainer::keyArea() const
{
    Q_D(const KeyAreaContainer);
    return d->key_area;
}


int KeyAreaContainer::width() const
{
    Q_D(const KeyAreaContainer);
    return d->key_area.rect().width();
}


int KeyAreaContainer::height() const
{
    Q_D(const KeyAreaContainer);
    return d->key_area.rect().height();
}


int KeyAreaContainer::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    Q_D(const KeyAreaContainer);
    return d->key_area.keys().count();
}


QVariant KeyAreaContainer::data(const QModelIndex &index,
                                int role) const
{
    Q_D(const KeyAreaContainer);

    const QVector<Key> &keys(d->key_area.keys());
    const Key &key(index.row() < keys.count()
                   ? keys.at(index.row())
                   : Key());

    switch(role) {
    case RoleKeyRect:
        return QVariant(key.rect());

    case RoleKeyBackground:
        return QVariant(key.area().background());

    case RoleKeyText:
        return QVariant(key.label().text());
    }

    qWarning() << __PRETTY_FUNCTION__
               << "Invalid index or role (" << index.row() << role << ").";

    return QVariant();
}


QVariant KeyAreaContainer::data(int index, const QString &role) const
{

    const QModelIndex idx(this->index(index, 0));
    return data(idx, roleNames().key(role.toLatin1()));
}


}} // namespace Model, MaliitKeyboard
