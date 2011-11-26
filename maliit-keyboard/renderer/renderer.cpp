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

#include "renderer.h"
#include "keyareaitem.h"

#ifdef MALIIT_KEYBOARD_HAVE_GL
#include <QGLWidget>
#endif

namespace MaliitKeyboard { namespace {

QGraphicsView * createView(QWidget *widget)
{
    QGraphicsView *view = new QGraphicsView(widget);
    view->resize(widget->size());
    QGraphicsScene *scene = new QGraphicsScene(view);
    view->setScene(scene);
    view->setFrameShape(QFrame::NoFrame);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setAttribute(Qt::WA_OpaquePaintEvent);
    view->setAttribute(Qt::WA_NoSystemBackground);

#ifdef MALIIT_KEYBOARD_HAVE_GL
    view->setViewport(new QGLWidget);
#endif
    view->viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    view->viewport()->setAttribute(Qt::WA_NoSystemBackground);

    return view;
}

QGraphicsItem * rootItem(QGraphicsView *view)
{
    if (not view || not view->scene()) {
        return 0;
    }

    QList<QGraphicsItem *> items = view->scene()->items(Qt::DescendingOrder);
    if (not items.isEmpty()) {
        return items.at(0);
    }

    return 0;
}

} // namespace

class RendererPrivate
{
public:
    QWidget *window;
    QGraphicsView *view;
    QHash<int, KeyAreaItem *> registry;

    explicit RendererPrivate()
        : window(0)
        , view(0)
        , registry()
    {}
};

Renderer::Renderer(QObject *parent)
    : QObject(parent)
    , d_ptr(new RendererPrivate)
{}

Renderer::~Renderer()
{}

void Renderer::setWindow(QWidget *window)
{
    Q_D(Renderer);
    d->window = window;
}

void Renderer::show(const KeyArea &ka)
{
    Q_D(Renderer);

    if (not d->window) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No main window specified, don't know where to render to.";
        return;
    }

    if (not d->view) {
        d->view = createView(d->window);
        d->view->showFullScreen();

        QGraphicsRectItem *root = new QGraphicsRectItem;
        root->setRect(d->view->rect());
        root->show();
        d->view->scene()->addItem(root);
   }

    // Allow for multiple key areas being shown at the same time:
    // TODO: Animate fade-in, from nearest window border.
    if (KeyAreaItem *found = d->registry.value(ka.id)) {
        found->show();
        found->update();
    } else {
        KeyAreaItem *item = new KeyAreaItem(&d->registry, ka, rootItem(d->view));
        item->show();
        item->update();
    }
}

void Renderer::hide(const KeyArea &ka)
{
    Q_D(Renderer);

    // TODO: Animate fade-out.
    // TODO: Animate fade-out, towards nearest window border.
    if (KeyAreaItem *found = d->registry.value(ka.id)) {
        found->hide();
    }
}

void Renderer::hideAll()
{}

void Renderer::setDelta(const KeyArea &ka)
{
    Q_UNUSED(ka)
}

} // namespace MaliitKeyboard
