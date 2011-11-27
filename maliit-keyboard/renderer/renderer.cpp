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
#include "graphicsview.h"

#ifdef MALIIT_KEYBOARD_HAVE_GL
#include <QGLWidget>
#endif

namespace MaliitKeyboard { namespace {

QGraphicsView * createView(QWidget *widget,
                           AbstractBackgroundBuffer *buffer)
{
    GraphicsView *view = new GraphicsView(widget);
    view->setBackgroundBuffer(buffer);
    view->resize(widget->size());
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    QGraphicsScene *scene = new QGraphicsScene(view);
    view->setScene(scene);
    view->setFrameShape(QFrame::NoFrame);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

#ifdef MALIIT_KEYBOARD_HAVE_GL
    view->setViewport(new QGLWidget);
#endif

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
    QScopedPointer<QGraphicsView> view;
    AbstractBackgroundBuffer *buffer;
    QHash<int, KeyAreaItem *> registry;
    QRegion region;

    explicit RendererPrivate()
        : window(0)
        , view(0)
        , buffer(0)
        , registry()
        , region()
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

    d->view.reset(createView(d->window, d->buffer));
    d->view->show();

    QGraphicsRectItem *root = new QGraphicsRectItem;
    root->setRect(d->view->rect());
    root->show();
    d->view->scene()->addItem(root);
}

void Renderer::setBackgroundBuffer(AbstractBackgroundBuffer *buffer)
{
    Q_D(Renderer);
    d->buffer = buffer;
}

QRegion Renderer::region() const
{
    Q_D(const Renderer);
    return d->region;
}

void Renderer::show(const KeyArea &ka)
{
    Q_D(Renderer);

    if (not d->window || d->view.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No main window specified, don't know where to render to.";
        return;
    }

    // Allows for multiple key areas being shown at the same time:
    // TODO: Animate fade-in, from nearest window border.
    KeyAreaItem *item = 0;
    if (KeyAreaItem *found = d->registry.value(ka.id)) {
        item = found;
    } else {
        item = new KeyAreaItem(&d->registry, rootItem(d->view.data()));
    }

    d->region -= QRegion(item->boundingRect().toRect());
    item->setKeyArea(ka);

    d->region |= QRegion(item->boundingRect().toRect());
    item->show();
}

void Renderer::hide(const KeyArea &ka)
{
    Q_D(Renderer);

    // TODO: Animate fade-out, towards nearest window border.
    if (KeyAreaItem *found = d->registry.value(ka.id)) {
        d->region -= QRegion(found->boundingRect().toRect());
        found->hide();
    }
}

void Renderer::hideAll()
{
    Q_D(Renderer);

    d->region = QRegion();
    foreach (KeyAreaItem *item, d->registry) {
        item->hide();
    }
}

void Renderer::setDelta(const KeyArea &ka)
{
    Q_UNUSED(ka)
}

} // namespace MaliitKeyboard
