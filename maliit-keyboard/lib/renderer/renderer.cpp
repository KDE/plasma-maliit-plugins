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
#include "keyitem.h"
#include "graphicsview.h"
#include "models/keyarea.h"

#ifdef MALIIT_KEYBOARD_HAVE_GL
#include <QGLWidget>
#endif

namespace MaliitKeyboard {

namespace {

class LayoutItem {
public:
    SharedLayout layout;
    KeyAreaItem *left_item;
    KeyAreaItem *right_item;
    KeyAreaItem *center_item;
    KeyAreaItem *extended_item;
    QRegion region;

    explicit LayoutItem()
        : layout()
        , left_item(0)
        , right_item(0)
        , center_item(0)
        , extended_item(0)
        , region()
    {}

    KeyAreaItem *activeItem() const
    {
        if (layout.isNull()) {
            qCritical() << __PRETTY_FUNCTION__
                        << "Invalid layout!";
            return 0;
        }

        switch(layout->activePanel()) {
        case Layout::LeftPanel:
            return left_item;

        case Layout::RightPanel:
            return right_item;

        case Layout::CenterPanel:
            return center_item;

        case Layout::ExtendedPanel:
            return extended_item;

        default:
            qCritical() << __PRETTY_FUNCTION__
                        << "Invalid case - should not be reached!"
                        << layout->activePanel();
            return 0;
        }

        qCritical() << __PRETTY_FUNCTION__
                    << "Should not be reached!";
        return 0;
    }

    void show(QGraphicsItem *root,
              QRegion *region)
    {
        if (layout.isNull() || not region) {
            qCritical() << __PRETTY_FUNCTION__
                        << "Invalid region or layout!";
            return;
        }

        if (not center_item) {
            center_item = new KeyAreaItem(root);
        }

        if (not extended_item) {
            extended_item = new KeyAreaItem(root);
        }

        center_item->setKeyArea(layout->centerPanel());
        center_item->show();
        *region |= QRegion(layout->centerPanel().rect.toRect());

        extended_item->setKeyArea(layout->extendedPanel());

        if (layout->activePanel() != Layout::ExtendedPanel) {
            extended_item->hide();
        } else {
            extended_item->show();
            *region |= QRegion(layout->extendedPanel().rect.toRect());
        }
    }

    void hide()
    {
        if (left_item) {
            left_item->hide();
        }

        if (right_item) {
            right_item->hide();
        }

        if (center_item) {
            center_item->hide();
        }

        if (extended_item) {
            extended_item->hide();
        }
    }
};

QGraphicsView * createView(QWidget *widget,
                           AbstractBackgroundBuffer *buffer)
{
    GraphicsView *view = new GraphicsView(widget);
    view->setBackgroundBuffer(buffer);
    view->resize(widget->size());
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    view->setOptimizationFlags(QGraphicsView::DontClipPainter | QGraphicsView::DontSavePainterState);
    QGraphicsScene *scene = new QGraphicsScene(view);
    view->setScene(scene);
    view->setFrameShape(QFrame::NoFrame);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

#ifdef MALIIT_KEYBOARD_HAVE_GL
    view->setViewport(new QGLWidget);
#endif

    scene->setSceneRect(widget->rect());
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

void recycleKeyItem(QVector<KeyItem *> *key_items,
                    int index,
                    const Key &key,
                    QGraphicsItem *parent)
{
    KeyItem *item = 0;

    if (index >= key_items->count()) {
        item = new KeyItem;
        key_items->append(item);
    } else {
        item = key_items->at(index);
    }

    item->setParentItem(parent);
    item->setKey(key);
    item->show();
}

} // namespace

class RendererPrivate
{
public:
    QWidget *window;
    QScopedPointer<QGraphicsView> view;
    AbstractBackgroundBuffer *buffer;
    QRegion region;
    QVector<LayoutItem> layout_items;
    QVector<KeyItem *> key_items;

    explicit RendererPrivate()
        : window(0)
        , view(0)
        , buffer(0)
        , region()
        , layout_items()
        , key_items()
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

QWidget * Renderer::viewport() const
{
    Q_D(const Renderer);
    return d->view->viewport();
}

void Renderer::addLayout(const SharedLayout &layout)
{
    Q_D(Renderer);

    LayoutItem li;
    li.layout = layout;
    d->layout_items.append(li);
}

void Renderer::clearLayouts()
{
    Q_D(Renderer);

    d->layout_items.clear();
    d->key_items.clear();
    d->view->scene()->clear();
}

void Renderer::show()
{
    Q_D(Renderer);

    if (not rootItem(d->view.data())) {
        d->view->scene()->addItem(new QGraphicsRectItem);
    }

    if (not d->view) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No view exists, aborting!";
    }

    for (int index = 0; index < d->layout_items.count(); ++index) {
        LayoutItem &li(d->layout_items[index]);
        li.show(rootItem(d->view.data()), &d->region);
    }

    d->view->show();
}

void Renderer::hide()
{
    Q_D(Renderer);

    foreach (LayoutItem li, d->layout_items) {
        li.hide();
    }

    d->region = QRegion();
}

void Renderer::onLayoutChanged(const SharedLayout &layout)
{
    Q_UNUSED(layout)
    show();
}

void Renderer::onKeysChanged(const SharedLayout &layout)
{
    if (layout.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Invalid layout.";
        return;
     }

    Q_D(Renderer);

    KeyAreaItem *parent = 0;
    for (int index = 0; index < d->layout_items.count(); ++index) {
        const LayoutItem &li(d->layout_items.at(index));

        if (li.layout == layout) {
            parent = li.activeItem();
            break;
        }
    }

    // Found the KeyAreaItem, which means layout is known by the renderer, too.
    if (parent) {
        const QVector<Key> &active_keys(layout->activeKeys());
        int index = 0;

        for (; index < active_keys.count(); ++index) {
            recycleKeyItem(&d->key_items, index, active_keys.at(index), parent);
        }

        if (layout->magnifierKey().valid()) {
            recycleKeyItem(&d->key_items, index, layout->magnifierKey(), parent);
            ++index;
        }

        // Hide remaining, currently unneeded key items:
        for (; index < d->key_items.count(); ++index) {
            d->key_items.at(index)->hide();
        }
    }
}

} // namespace MaliitKeyboard
