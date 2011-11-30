/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
 *
 */

#include "glass.h"

namespace MaliitKeyboard {

class GlassPrivate
{
public:
    QWidget *window;
    QVector<SharedKeyArea> key_areas;
    QVector<Key> pressed_keys;
};

Glass::Glass(QObject *parent)
    : QObject(parent)
    , d_ptr(new GlassPrivate)
{}

Glass::~Glass()
{}

void Glass::setWindow(QWidget *window)
{
    Q_D(Glass);

    d->window = window;
    deactivateAll();

    d->window->installEventFilter(this);
}

void Glass::activate(const SharedKeyArea &ka)
{
    if (ka.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Cannot activate non-existant KeyArea.";
        return;
    }

    Q_D(Glass);

    d->key_areas.append(ka);
}

void Glass::deactivate(const SharedKeyArea &ka)
{
    if (ka.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Cannot deactivate non-existant KeyArea.";
        return;
    }

    Q_D(Glass);

    for (int index = 0; index < d->key_areas.count(); ++index) {
        if (ka == d->key_areas.at(index)) {
            d->key_areas.remove(index);
            break;
        }
    }
}

void Glass::deactivateAll()
{
    Q_D(Glass);
    d->key_areas.clear();
}

bool Glass::eventFilter(QObject *obj,
                        QEvent *ev)
{
    // FIXME: Remove test code
    bool static init = false;
        static QPixmap pressed_bg(8, 8);

    if (not init) {
        pressed_bg.fill(Qt::darkBlue);
        init = true;
    }

    if (not obj || not ev) {
        return false;
    }

    Q_D(Glass);

    switch(ev->type()) {
    case QKeyEvent::MouseButtonPress:
    case QKeyEvent::MouseButtonRelease: {
        QMouseEvent *qme = static_cast<QMouseEvent *>(ev);
        ev->accept();

        foreach (const SharedKeyArea &ka, d->key_areas) {
            const QVector<Key> &keys(ka->keys());
            // FIXME: use binary range search
            if (ka->rect().contains(qme->posF())) {
                bool ka_changed = false;
                for (int index = 0; index < keys.count(); ++index) {
                    Key k(keys.at(index));

                    // Pressed state is not stored in key itself, so list of pressed keys (overridden keys) must be maintained elsewhere.
                    const bool pressed = k.rect().translated(ka->rect().toRect().topLeft()).contains(qme->pos());
                    const bool was_pressed(ka->activeKeys().contains(k));

                    if (was_pressed && pressed) {
                        ka->removeFromActiveKeys(k);
                        ka_changed = true;
                    } else if (not was_pressed && pressed) {
                        k.setBackground(pressed_bg);
                        ka->appendToActiveKeys(k);
                        ka_changed = true;
                    }
                }

                if (ka_changed) {
                    emit keyAreaChanged(ka, KeyArea::ActiveKeysChanged);
                }
            }
        }

        return true;
    }

    default:
        break;
    }

    return false;
}

} // namespace MaliitKeyboard
