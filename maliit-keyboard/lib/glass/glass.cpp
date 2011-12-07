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
    QVector<SharedLayout> layouts;

    explicit GlassPrivate()
        : window(0)
        , layouts()
    {}
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
    clearLayouts();

    d->window->installEventFilter(this);
}

void Glass::addLayout(const SharedLayout &layout)
{
    Q_D(Glass);
    d->layouts.append(layout);
}

void Glass::clearLayouts()
{
    Q_D(Glass);
    d->layouts.clear();
}

bool Glass::eventFilter(QObject *obj,
                        QEvent *ev)
{
    Q_D(Glass);
    static bool measure_fps(QCoreApplication::arguments().contains("-measure-fps"));

    if (not obj || not ev) {
        return false;
    }

    switch(ev->type()) {
    case QEvent::Paint: {
        if (measure_fps) {
            static int count = 0;
            static QElapsedTimer fps_timer;

            if (0 == count % 120) {
                qDebug() << "FPS:" << count / ((0.01 + fps_timer.elapsed()) / 1000) << count;
                fps_timer.restart();
                count = 0;
            }

            d->window->update();
            ++count;
        }
    } break;

    case QKeyEvent::MouseButtonPress:
    case QKeyEvent::MouseButtonRelease: {
        QMouseEvent *qme = static_cast<QMouseEvent *>(ev);
        ev->accept();

        foreach (const SharedLayout &layout, d->layouts) {
            const QPoint &pos(layout->orientation() == Layout::Landscape
                              ? qme->pos() : QPoint(d->window->height() - qme->pos().y(), qme->pos().x()));
            const QVector<Key> &keys(layout->centerPanel().keys);

            // FIXME: use binary range search
            const QRect &rect(layout->activeKeyArea().rect.toRect());
            if (rect.contains(pos)) {

                for (int index = 0; index < keys.count(); ++index) {
                    Key k(keys.at(index));

                    // Pressed state is not stored in key itself, so list of pressed keys (overridden keys) must be maintained elsewhere.
                    const QRect &key_rect = k.rect().translated(rect.topLeft());
                    if (key_rect.contains(pos)) {

                        if (qme->type() == QKeyEvent::MouseButtonPress) {
                            emit keyPressed(k, layout);
                        } else if (qme->type() == QKeyEvent::MouseButtonRelease) {
                            emit keyReleased(k, layout);
                        }

                        break;
                    }
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
