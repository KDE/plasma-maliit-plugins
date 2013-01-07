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
#include "keydescription.h"

#include "logic/layout.h"
#include "logic/layoutupdater.h"

namespace MaliitKeyboard {
namespace Model {
namespace {
QUrl toUrl(const QString &directory,
            const QString &base_name)
{
    if (not (directory.isEmpty() || base_name.isEmpty())) {
        return QUrl(directory + "/" + base_name);
    }

    return QUrl();

}
}


class KeyAreaContainerPrivate
{
public:
    KeyArea key_area;
    Logic::Layout *layout; // TODO: Get rid of this member.
    Logic::LayoutUpdater *updater; // TODO: wrap into scoped pointer and assign ownership to this class.
    QString image_directory;
    QHash<int, QByteArray> roles;

    explicit KeyAreaContainerPrivate(Logic::LayoutUpdater *new_updater);
};


KeyAreaContainerPrivate::KeyAreaContainerPrivate(Logic::LayoutUpdater *new_updater)
    : key_area()
    , layout()
    , updater(new_updater)
    , image_directory()
    , roles()
{
    // Model roles are used as variables in QML, hence the under_score naming
    // convention:
    roles[KeyAreaContainer::RoleKeyRectangle] = "key_rectangle";
    roles[KeyAreaContainer::RoleKeyReactiveArea] = "key_reactive_area";
    roles[KeyAreaContainer::RoleKeyBackground] = "key_background";
    roles[KeyAreaContainer::RoleKeyBackgroundBorders] = "key_background_borders";
    roles[KeyAreaContainer::RoleKeyText] = "key_text";
}


KeyAreaContainer::KeyAreaContainer(Logic::LayoutUpdater *updater,
                                   QObject *parent)
    : QAbstractListModel(parent)
    , d_ptr(new KeyAreaContainerPrivate(updater))
{}


KeyAreaContainer::~KeyAreaContainer()
{}


void KeyAreaContainer::setKeyArea(const KeyArea &area)
{
    beginResetModel();

    Q_D(KeyAreaContainer);
    const bool geometry_changed(d->key_area.rect() != area.rect());
    const bool background_changed(d->key_area.area().background() != area.area().background());

    d->key_area = area;

    if (geometry_changed) {
        Q_EMIT widthChanged(width());
        Q_EMIT heightChanged(height());
    }

    if (background_changed) {
        Q_EMIT backgroundChanged(background());
    }

    endResetModel();
}


KeyArea KeyAreaContainer::keyArea() const
{
    Q_D(const KeyAreaContainer);
    return d->key_area;
}


void KeyAreaContainer::setLayout(Logic::Layout *layout)
{
    Q_D(KeyAreaContainer);

    d->layout = layout;
}


Logic::Layout *KeyAreaContainer::layout() const
{
    Q_D(const KeyAreaContainer);

    return d->layout;
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


QUrl KeyAreaContainer::background() const
{
    Q_D(const KeyAreaContainer);
    return toUrl(d->image_directory, d->key_area.area().background());
}


void KeyAreaContainer::setImageDirectory(const QString &directory)
{
    Q_D(KeyAreaContainer);

    if (d->image_directory != directory) {
        d->image_directory = directory;
        // TODO: Make sure we don't accidentially invalidate the whole model twice
        beginResetModel();
        backgroundChanged(background());
        endResetModel();
    }
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
    case RoleKeyReactiveArea:
        return QVariant(key.rect());

    case RoleKeyRectangle: {
        const QRect &r(key.rect());
        const QMargins &m(key.margins());

        return QVariant(QRectF(m.left(), m.top(),
                               r.width() - (m.left() + m.right()),
                               r.height() - (m.top() + m.bottom())));
    }

    case RoleKeyBackground:
        return QVariant(toUrl(d->image_directory, key.area().background()));

    case RoleKeyBackgroundBorders: {
        // Neither QML nor QVariant support QMargins type.
        // We need to transform QMargins into a QRectF so that we can abuse
        // left, top, right, bottom (of the QRectF) *as if* it was a QMargins.
        const QMargins &m(key.area().backgroundBorders());
        return QVariant(QRectF(m.left(), m.top(), m.right(), m.bottom()));
    }

    case RoleKeyText:
        return QVariant(key.label().text());
    }

    qWarning() << __PRETTY_FUNCTION__
               << "Invalid index or role (" << index.row() << role << ").";

    return QVariant();
}


QHash<int, QByteArray> KeyAreaContainer::roleNames() const
{
    Q_D(const KeyAreaContainer);
    return d->roles;
}


QVariant KeyAreaContainer::data(int index, const QString &role) const
{

    const QModelIndex idx(this->index(index, 0));
    return data(idx, roleNames().key(role.toLatin1()));
}


void KeyAreaContainer::onEntered(int index)
{
    Q_D(KeyAreaContainer);

    const QVector<Key> &keys(d->key_area.keys());
    const Key &key(index < keys.count()
                   ? keys.at(index)
                   : Key());

    if (d->updater) {
        d->updater->onKeyEntered(key);
    }

    Q_EMIT keyEntered(key);
}


void KeyAreaContainer::onExited(int index)
{
    Q_D(KeyAreaContainer);

    const QVector<Key> &keys(d->key_area.keys());
    const Key &key(index < keys.count()
                   ? keys.at(index)
                   : Key());

    if (d->updater) {
        d->updater->onKeyExited(key);
    }

    Q_EMIT keyExited(key);
}


void KeyAreaContainer::onPressed(int index)
{
    Q_D(KeyAreaContainer);

    const QVector<Key> &keys(d->key_area.keys());
    const Key &key(index < keys.count()
                   ? keys.at(index) : Key());
    const Key pressed_key(d->updater
                         ? d->updater->modifyKey(key, KeyDescription::PressedState) : Key());

    d->key_area.rKeys().replace(index, pressed_key);

    if (d->updater) {
        d->updater->onKeyPressed(pressed_key);
    }

    Q_EMIT dataChanged(this->index(index, 0), this->index(index, 0));
    Q_EMIT keyPressed(pressed_key);
}


void KeyAreaContainer::onReleased(int index)
{
    Q_D(KeyAreaContainer);

    const QVector<Key> &keys(d->key_area.keys());
    const Key &key(index < keys.count()
                   ? keys.at(index) : Key());
    const Key normal_key(d->updater
                         ? d->updater->modifyKey(key, KeyDescription::NormalState) : Key());

    d->key_area.rKeys().replace(index, normal_key);

    if (d->updater) {
        d->updater->onKeyReleased(normal_key);
    }

    Q_EMIT dataChanged(this->index(index, 0), this->index(index, 0));
    Q_EMIT keyReleased(normal_key);
}


void KeyAreaContainer::onPressAndHold(int index)
{
    Q_D(KeyAreaContainer);

    const QVector<Key> &keys(d->key_area.keys());
    const Key &key(index < keys.count()
                   ? keys.at(index)
                   : Key());

    if (d->updater) {
        d->updater->onKeyLongPressed(key);
    }

    Q_EMIT keyLongPressed(key);
}


}} // namespace Model, MaliitKeyboard
