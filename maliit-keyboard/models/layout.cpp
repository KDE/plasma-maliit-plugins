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

Layout::Layout()
    : m_active_panel(Layout::CenterPanel)
    , m_left()
    , m_right()
    , m_center()
    , m_extended()
{}

Layout::Panel Layout::activePanel() const
{
    return m_active_panel;
}

void Layout::setActivePanel(Panel panel)
{
    m_active_panel = panel;
}

KeyArea Layout::activeKeyArea() const
{
    return lookup(activePanel());
}

KeyArea Layout::leftPanel() const
{
    return m_left;
}

void Layout::setLeftPanel(const KeyArea &left)
{
    if (m_left != left) {
        m_left = left;
        m_active_keys.left.clear();
    }
}

KeyArea Layout::rightPanel() const
{
    return m_right;
}

void Layout::setRightPanel(const KeyArea &right)
{
    if (m_right != right) {
        m_right = right;
        m_active_keys.right.clear();
    }
}

KeyArea Layout::centerPanel() const
{
    return m_center;
}

void Layout::setCenterPanel(const KeyArea &center)
{
    if (m_center != center) {
        m_center = center;
        m_active_keys.center.clear();
    }
}

KeyArea Layout::extendedPanel() const
{
    return m_extended;
}

void Layout::setExtendedPanel(const KeyArea &extended)
{
    if (m_extended != extended) {
        m_extended = extended;
        m_active_keys.extended.clear();
    }
}

QVector<Key> Layout::activeKeys() const
{
    switch (m_active_panel) {
    case LeftPanel: return m_active_keys.left;
    case RightPanel: return m_active_keys.right;
    case CenterPanel: return m_active_keys.center;
    case ExtendedPanel: return m_active_keys.extended;
    case PanelCount: break;
    }

    static const QVector<Key> empty;
    return empty;
}

void Layout::appendActiveKey(const Key &key)
{
    switch (m_active_panel) {
    case LeftPanel: m_active_keys.left.append(key); break;
    case RightPanel: m_active_keys.right.append(key); break;
    case CenterPanel: m_active_keys.center.append(key); break;
    case ExtendedPanel: m_active_keys.extended.append(key); break;
    case PanelCount: break;
    }
}

void Layout::removeActiveKey(const Key &key)
{
    QVector<Key> *active_keys = 0;
    switch (m_active_panel) {
    case LeftPanel: active_keys = &m_active_keys.left; break;
    case RightPanel: active_keys = &m_active_keys.right; break;
    case CenterPanel: active_keys = &m_active_keys.center; break;
    case ExtendedPanel: active_keys = &m_active_keys.extended; break;
    case PanelCount: break;
    }

    if (active_keys) {
        for (int index = 0; index < active_keys->count(); ++index) {
            if (active_keys->at(index) == key) {
                active_keys->remove(index);
                break;
            }
        }
    }
}

KeyArea Layout::lookup(Panel panel) const
{
    switch(panel) {
    case LeftPanel: return m_left;
    case RightPanel: return m_right;
    case CenterPanel: return m_center;
    case ExtendedPanel: return m_extended;
    case PanelCount: break;
    }

    qCritical() << __PRETTY_FUNCTION__
                << "Should not be reached, invalid panel:" << panel;
    static const KeyArea invalid;
    return invalid;
}

} // namespace MaliitKeyboard
