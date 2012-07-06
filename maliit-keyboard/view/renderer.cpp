/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 * Copyright (C) 2012 Openismus GmbH
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
#include "wordribbonitem.h"
#include "graphicsview.h"
#include "utils.h"

#include "models/keyarea.h"
#include "models/wordribbon.h"

#ifdef MALIIT_KEYBOARD_HAVE_OPENGL
#include <QGLWidget>
#endif

#include <maliit/plugins/abstractwidgetssurface.h>

using Maliit::Plugins::AbstractGraphicsViewSurface;
using Maliit::Plugins::AbstractSurface;
using Maliit::Plugins::AbstractSurfaceFactory;

namespace MaliitKeyboard {

namespace {

const qreal WordRibbonZIndex = 0.0f;
const qreal CenterPanelZIndex = 1.0f;
const qreal ExtendedPanelZIndex = 20.0f;
const qreal ActiveKeyZIndex = 0.0f;

QGraphicsView * createView(QWidget *widget,
                           AbstractBackgroundBuffer *buffer)
{
    GraphicsView *view = new GraphicsView(widget);
    view->setBackgroundBuffer(buffer);
    view->resize(widget->size());
    view->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    view->setOptimizationFlags(QGraphicsView::DontClipPainter | QGraphicsView::DontSavePainterState);
    QGraphicsScene *scene = new QGraphicsScene(view);
    view->setScene(scene);
    view->setSceneRect(widget->rect());
    view->setFrameShape(QFrame::NoFrame);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

#ifdef MALIIT_KEYBOARD_HAVE_OPENGL
    QGLWidget *gl_widget = new QGLWidget;
    if (gl_widget->isValid()) {
        view->setViewport(gl_widget);
        view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    } else {
        delete gl_widget;
    }
#endif

    // If there is no buffer, it probably means we run stand-alone. But when
    // run as a maliit-server plugin, the server takes care of making any
    // background widget visible. Without the server, we have to make the
    // QGraphicsView translucent ourselves:
    if (not buffer) {
        view->setBackgroundBrush(Qt::transparent);
        view->setBackgroundRole(QPalette::NoRole);
        view->setWindowFlags(Qt::FramelessWindowHint);
        view->setAttribute(Qt::WA_NoSystemBackground);
        view->viewport()->setAutoFillBackground(false);
    }

    scene->setSceneRect(widget->rect());
    view->show();

    return view;
}

void recycleKeyItem(QVector<KeyItem *> *key_items,
                    int index,
                    const Key &key,
                    QGraphicsItem *parent)
{
    KeyItem *item = 0;

    if (index >= key_items->count()) {
        item = new KeyItem;
        item->setZValue(ActiveKeyZIndex);
        key_items->append(item);
    } else {
        item = key_items->at(index);
    }

    item->setParentItem(parent);
    item->setKey(key);
    item->show();
}

void updateActiveKeys(const QVector<Key> &active_keys,
                      QVector<KeyItem *> *key_items,
                      QGraphicsItem *parent)
{
    int index = 0;

    for (; index < active_keys.count(); ++index) {
        recycleKeyItem(key_items, index, active_keys.at(index), parent);
    }

    // Hide remaining, currently unneeded key items:
    for (; index < key_items->count(); ++index) {
        key_items->at(index)->hide();
    }
}

} // namespace

class RendererPrivate
{
public:
    Maliit::Plugins::AbstractSurfaceFactory *factory;
    QSharedPointer<Maliit::Plugins::AbstractGraphicsViewSurface> surface;
    QSharedPointer<Maliit::Plugins::AbstractGraphicsViewSurface> extended_surface;
    QSharedPointer<Maliit::Plugins::AbstractGraphicsViewSurface> magnifier_surface;
    SharedStyle style;
    KeyAreaItem *center_item;
    KeyAreaItem *extended_item;
    WordRibbonItem *ribbon_item;
    QVector<KeyItem *> key_items;
    QVector<KeyItem *> extended_key_items;
    QVector<KeyItem *> magnifier_key_items;
    QString images_directory_path;

    explicit RendererPrivate()
        : factory(0)
        , surface()
        , extended_surface()
        , magnifier_surface()
        , center_item(new KeyAreaItem)
        , extended_item(new KeyAreaItem)
        , ribbon_item(new WordRibbonItem)
        , key_items()
        , extended_key_items()
        , magnifier_key_items()
    {
        center_item->setZValue(CenterPanelZIndex);
        extended_item->setZValue(ExtendedPanelZIndex);
        ribbon_item->setZValue(WordRibbonZIndex);
    }
};

Renderer::Renderer(QObject *parent)
    : QObject(parent)
    , d_ptr(new RendererPrivate)
{}

Renderer::~Renderer()
{}

void Renderer::setSurfaceFactory(AbstractSurfaceFactory *factory)
{
    Q_D(Renderer);
    d->factory = factory;

    // Assuming that factory == 0 means reset:
    if (not d->factory) {
        // Drop references => shared surface instances are eventually deleted:
        d->surface.clear();
        d->extended_surface.clear();
        d->magnifier_surface.clear();
        return;
    }

    d->surface = qSharedPointerDynamicCast<AbstractGraphicsViewSurface>(
        factory->create(AbstractSurface::PositionCenterBottom | AbstractSurface::TypeGraphicsView));

    d->extended_surface = qSharedPointerDynamicCast<AbstractGraphicsViewSurface>(
        factory->create(AbstractSurface::PositionOverlay | AbstractSurface::TypeGraphicsView, d->surface));

    d->magnifier_surface = qSharedPointerDynamicCast<AbstractGraphicsViewSurface>(
        factory->create(AbstractSurface::PositionOverlay | AbstractSurface::TypeGraphicsView, d->surface));
}

const QSharedPointer<Maliit::Plugins::AbstractGraphicsViewSurface> Renderer::surface() const
{
    Q_D(const Renderer);

    return d->surface;
}

const QSharedPointer<Maliit::Plugins::AbstractGraphicsViewSurface> Renderer::extendedSurface() const
{
    Q_D(const Renderer);

    return d->extended_surface;
}

void Renderer::setStyle(const SharedStyle &style)
{
    Q_D(Renderer);
    if (d->style != style) {
        if (d->style) {
            disconnect(d->style.data(), SIGNAL(profileChanged()),
                       this,            SLOT(applyProfile()));
        }
        d->style = style;
        connect(d->style.data(), SIGNAL(profileChanged()),
                this,            SLOT(applyProfile()));
        applyProfile();
    }
}

void Renderer::show()
{
    Q_D(Renderer);

    if (d->surface.isNull()
        || d->extended_surface.isNull()
        || d->magnifier_surface.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Some surfaces not available, cannot show keyboard!"
                    << "Discarding show request.";
        return;
    }

    d->surface->show();
    d->extended_surface->show();
    d->magnifier_surface->show();

    // The root item of a surface only becomes valid after being shown (for
    // the first time):
    d->center_item->setParentItem(d->surface->root());
    d->extended_item->setParentItem(d->extended_surface->root());
    d->ribbon_item->setParentItem(d->surface->root());
}

void Renderer::hide()
{
    Q_D(Renderer);

    if (d->surface.isNull()
        || d->extended_surface.isNull()
        || d->magnifier_surface.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Some surfaces not available, cannot hide keyboard!"
                    << "Discarding hide request.";
        return;
    }

    d->surface->hide();
    d->extended_surface->hide();
    d->magnifier_surface->hide();
}

void Renderer::onActiveKeysChanged(const QVector<Key> &active_keys)
{
    Q_D(Renderer);
    updateActiveKeys(active_keys, &d->key_items, d->center_item);
}

void Renderer::onActiveExtendedKeysChanged(const QVector<Key> &active_keys)
{
    Q_D(Renderer);
    updateActiveKeys(active_keys, &d->extended_key_items, d->extended_item);
}

void Renderer::onMagnifierKeyChanged(const Key &key)
{
    Q_D(Renderer);

    if (key.valid()) {
        Key magnifier_key(key);
        d->magnifier_surface->setSize(magnifier_key.area().size());
        d->magnifier_surface->setRelativePosition(magnifier_key.origin());
        magnifier_key.setOrigin(QPoint());
        d->magnifier_surface->show();
        recycleKeyItem(&d->magnifier_key_items, 0, magnifier_key, d->magnifier_surface->root());
    } else {
        d->magnifier_surface->hide();
    }
}

void Renderer::onCenterPanelChanged(const KeyArea &key_area,
                                    const QPoint &origin)
{
    Q_D(Renderer);

    const QSize &ka_size(key_area.area().size());
    d->center_item->setKeyArea(key_area, QRect(origin, ka_size));
    d->center_item->setVisible(ka_size.isValid());

    if (d->center_item->isVisible()) {
        d->surface->setSize(QSize(ka_size.width() + origin.x(),
                                  ka_size.height() + origin.y()));
        d->surface->show();
    } else {
        d->surface->hide();
    }
}

void Renderer::onExtendedPanelChanged(const KeyArea &key_area,
                                      const QPoint &origin)
{
    Q_D(Renderer);

    const QSize &ka_size(key_area.area().size());
    d->extended_item->setKeyArea(key_area, QRect(QPoint(), ka_size));
    d->extended_item->setVisible(ka_size.isValid());

    if (d->extended_item->isVisible()) {
        d->extended_surface->setSize(ka_size);
        d->extended_surface->setRelativePosition(origin);
        d->extended_surface->show();
    } else {
        d->extended_surface->hide();
    }
}

void Renderer::onWordRibbonChanged(const WordRibbon &ribbon,
                                   const QRect &geometry)
{
    Q_D(Renderer);

    d->ribbon_item->setWordRibbon(ribbon, geometry);
    d->ribbon_item->setVisible(geometry.isValid());
}

void Renderer::applyProfile()
{
    Q_D(Renderer);
    Utils::setImagesDirectoryPath(d->style->directoryPath(Style::Images));
}

} // namespace MaliitKeyboard
