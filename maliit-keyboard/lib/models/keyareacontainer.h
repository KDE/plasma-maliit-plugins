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

#ifndef MALIIT_KEYBOARD_KEYAREACONTAINER_H
#define MALIIT_KEYBOARD_KEYAREACONTAINER_H

#include <QtCore>

#include "models/key.h"
#include "logic/layout.h"

namespace MaliitKeyboard {

class KeyArea;

namespace Model {

class KeyAreaContainerPrivate;

// TODO: Move the important/remaining layout handling features from
// Logic::Layout into this, effectively merging the two classes.
class KeyAreaContainer
    : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(KeyAreaContainer)
    Q_DECLARE_PRIVATE(KeyAreaContainer)

    Q_PROPERTY(int width READ width
                         NOTIFY widthChanged)
    Q_PROPERTY(int height READ height
                          NOTIFY heightChanged)
    Q_PROPERTY(QUrl background READ background
                               NOTIFY backgroundChanged)

public:
    enum Roles {
        RoleKeyRectangle = Qt::UserRole + 1,
        RoleKeyReactiveArea,
        RoleKeyBackground,
        RoleKeyBackgroundBorders,
        RoleKeyText,
    };

    explicit KeyAreaContainer(QObject *parent = 0);
    virtual ~KeyAreaContainer();

    Q_SLOT void setKeyArea(const KeyArea &area);
    KeyArea keyArea() const;

    void setLayout(Logic::Layout *layout);
    Logic::Layout *layout() const;

    Q_SLOT int width() const;
    Q_SIGNAL void widthChanged(int changed);

    Q_SLOT int height() const;
    Q_SIGNAL void heightChanged(int changed);

    Q_SLOT QUrl background() const;
    Q_SIGNAL void backgroundChanged(const QUrl &changed);

    Q_SLOT void setImageDirectory(const QString &directory);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index,
                  int role) const;
    Q_INVOKABLE QVariant data(int index,
                              const QString &role) const;

    Q_INVOKABLE void onEntered(int index);
    Q_INVOKABLE void onExited(int index);
    Q_INVOKABLE void onPressed(int index);
    Q_INVOKABLE void onReleased(int index);
    Q_INVOKABLE void onPressAndHold(int index);

    // Key signals:
    Q_SIGNAL void keyPressed(const Key &key,
                             Logic::Layout *layout);
    Q_SIGNAL void keyLongPressed(const Key &key,
                                 Logic::Layout *layout);
    Q_SIGNAL void keyReleased(const Key &key,
                              Logic::Layout *layout);
    Q_SIGNAL void keyEntered(const Key &key,
                             Logic::Layout *layout);
    Q_SIGNAL void keyExited(const Key &key,
                            Logic::Layout *layout);

private:
    const QScopedPointer<KeyAreaContainerPrivate> d_ptr;
};

}} // namespace Model, MaliitKeyboard

#endif // MALIIT_KEYBOARD_KEYAREACONTAINER_H
