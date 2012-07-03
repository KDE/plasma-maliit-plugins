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

#include "layout.h"

namespace MaliitKeyboard {

class LayoutPrivate
{
public:
    QSize screen_size;
    QPoint origin;
    QPoint extended_panel_offset;
    Layout::Orientation orientation;
    Layout::Alignment alignment;
    Layout::Panel active_panel;
    KeyArea left;
    KeyArea right;
    KeyArea center;
    KeyArea extended;
    WordRibbon ribbon;

    struct {
        QVector<Key> left;
        QVector<Key> right;
        QVector<Key> center;
        QVector<Key> extended;
    } active_keys;

    Key magnifier_key;
    QPoint magnifier_key_origin;

    explicit LayoutPrivate();
};

LayoutPrivate::LayoutPrivate()
    : screen_size()
    , extended_panel_offset()
    , orientation(Layout::Landscape)
    , alignment(Layout::Bottom)
    , active_panel(Layout::CenterPanel)
    , left()
    , right()
    , center()
    , extended()
    , ribbon()
    , active_keys()
    , magnifier_key()
    , magnifier_key_origin()
{}

Layout::Layout(QObject *parent)
    : QObject(parent)
    , d_ptr(new LayoutPrivate)
{}

Layout::~Layout()
{}

QSize Layout::screenSize() const
{
    Q_D(const Layout);
    return d->screen_size;
}

void Layout::setScreenSize(const QSize &size)
{
    Q_D(Layout);
    d->screen_size = size;
}

QPoint Layout::extendedPanelOffset() const
{
    Q_D(const Layout);
    return d->extended_panel_offset;
}

void Layout::setExtendedPanelOffset(const QPoint &offset)
{
    Q_D(Layout);
    d->extended_panel_offset = offset;
}

Layout::Orientation Layout::orientation() const
{
    Q_D(const Layout);
    return d->orientation;
}

void Layout::setOrientation(Orientation orientation)
{
    Q_D(Layout);
    d->orientation = orientation;
}

Layout::Alignment Layout::alignment() const
{
    Q_D(const Layout);
    return d->alignment;
}

void Layout::setAlignment(Alignment alignment)
{
    Q_D(Layout);
    d->alignment = alignment;
}

Layout::Panel Layout::activePanel() const
{
    Q_D(const Layout);
    return d->active_panel;
}

void Layout::setActivePanel(Panel panel)
{
    if (panel != NumPanels) {
        Q_D(Layout);
        d->active_panel = panel;
    }
}

QRect Layout::geometry(Panel panel) const
{
    Q_UNUSED(panel)
    return QRect();
}

KeyArea Layout::activeKeyArea() const
{
    return lookup(activePanel());
}

void Layout::setActiveKeyArea(const KeyArea &active)
{
    switch (activePanel()) {
    case LeftPanel: setLeftPanel(active); break;
    case RightPanel: setRightPanel(active); break;
    case CenterPanel: setCenterPanel(active); break;
    case ExtendedPanel: setExtendedPanel(active); break;

    default:
        qCritical() << __PRETTY_FUNCTION__
                    << "Should not be reached, invalid panel:" << activePanel();
        break;
    }
}

QRect Layout::activeKeyAreaGeometry() const
{
    Q_D(const Layout);

    switch(d->active_panel) {
    case LeftPanel: return leftPanelGeometry();
    case RightPanel: return rightPanelGeometry();
    case CenterPanel: return centerPanelGeometry();
    case ExtendedPanel: return extendedPanelGeometry();

    default:
        break;
    }

    qCritical() << __PRETTY_FUNCTION__
                << "Should not be reached, invalid panel:" << d->active_panel;
    return QRect();
}

KeyArea Layout::leftPanel() const
{
    Q_D(const Layout);
    return d->left;
}

void Layout::setLeftPanel(const KeyArea &left)
{
    Q_D(Layout);

    if (d->left != left) {
        d->left = left;
    }
}

QRect Layout::leftPanelGeometry() const
{
    return QRect();
}

KeyArea Layout::rightPanel() const
{
    Q_D(const Layout);
    return d->right;
}

void Layout::setRightPanel(const KeyArea &right)
{
    Q_D(Layout);

    if (d->right != right) {
        d->right = right;
    }
}

QRect Layout::rightPanelGeometry() const
{
    return QRect();
}

KeyArea Layout::centerPanel() const
{
    Q_D(const Layout);
    return d->center;
}

void Layout::setCenterPanel(const KeyArea &center)
{
    Q_D(Layout);

    if (d->center != center) {
        d->center = center;
    }
}

QRect Layout::centerPanelGeometry() const
{
    Q_D(const Layout);
    return QRect(panelOrigin(), d->center.area().size());
}

KeyArea Layout::extendedPanel() const
{
    Q_D(const Layout);
    return d->extended;
}

void Layout::setExtendedPanel(const KeyArea &extended)
{
    Q_D(Layout);

    if (d->extended != extended) {
        d->extended = extended;
    }
}

QRect Layout::extendedPanelGeometry() const
{
    Q_D(const Layout);
    return QRect(QPoint(), d->extended.area().size());
}

QPoint Layout::extendedPanelOrigin() const
{
    Q_D(const Layout);
    return panelOrigin() + d->extended_panel_offset;
}

WordRibbon Layout::wordRibbon() const
{
    Q_D(const Layout);
    return d->ribbon;
}

void Layout::setWordRibbon(const WordRibbon &ribbon)
{
    Q_D(Layout);
    d->ribbon = ribbon;
}

QRect Layout::wordRibbonGeometry() const
{
    Q_D(const Layout);
    return QRect(origin(), d->ribbon.area().size());
}

QVector<Key> Layout::activeKeys() const
{
    Q_D(const Layout);

    switch (d->active_panel) {
    case LeftPanel: return d->active_keys.left;
    case RightPanel: return d->active_keys.right;
    case CenterPanel: return d->active_keys.center;
    case ExtendedPanel: return d->active_keys.extended;
    case NumPanels: break;
    }

    return QVector<Key>();
}

void Layout::clearActiveKeys()
{
    Q_D(Layout);

    d->active_keys.left.clear();
    d->active_keys.right.clear();
    d->active_keys.center.clear();
    d->active_keys.extended.clear();
}

void Layout::appendActiveKey(const Key &key)
{
    Q_D(Layout);

    switch (d->active_panel) {
    case LeftPanel: d->active_keys.left.append(key); break;
    case RightPanel: d->active_keys.right.append(key); break;
    case CenterPanel: d->active_keys.center.append(key); break;
    case ExtendedPanel: d->active_keys.extended.append(key); break;
    case NumPanels: break;
    }
}

void Layout::removeActiveKey(const Key &key)
{
    Q_D(Layout);

    QVector<Key> *active_keys = 0;
    switch (d->active_panel) {
    case LeftPanel: active_keys = &d->active_keys.left; break;
    case RightPanel: active_keys = &d->active_keys.right; break;
    case CenterPanel: active_keys = &d->active_keys.center; break;
    case ExtendedPanel: active_keys = &d->active_keys.extended; break;
    case NumPanels: break;
    }

    if (active_keys) {
        for (int index = 0; index < active_keys->count(); ++index) {
            const Key &current(active_keys->at(index));
            if (current.origin() == key.origin()
                && current.label() == key.label()) {
                active_keys->remove(index);
                break;
            }
        }
    }
}

Key Layout::magnifierKey() const
{
    Q_D(const Layout);
    return d->magnifier_key;
}

QPoint Layout::magnifierKeyOrigin() const
{
    Q_D(const Layout);
    return d->magnifier_key_origin;
}

void Layout::setMagnifierKey(const Key &key)
{
    Q_D(Layout);

    d->magnifier_key = key;
    d->magnifier_key_origin = d->magnifier_key.origin() + panelOrigin();
    d->magnifier_key.setOrigin(QPoint());
}

void Layout::clearMagnifierKey()
{
    setMagnifierKey(Key());
}

KeyArea Layout::lookup(Panel panel) const
{
    Q_D(const Layout);

    switch(panel) {
    case LeftPanel: return d->left;
    case RightPanel: return d->right;
    case CenterPanel: return d->center;
    case ExtendedPanel: return d->extended;
    case NumPanels: break;
    }

    qCritical() << __PRETTY_FUNCTION__
                << "Should not be reached, invalid panel:" << panel;
    return KeyArea();
}

QPoint Layout::origin() const
{
    return QPoint();
}

QPoint Layout::panelOrigin() const
{
    Q_D(const Layout);
    return (origin() + QPoint(0, d->ribbon.area().size().height()));
}

} // namespace MaliitKeyboard
